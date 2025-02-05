//apt install libsdl2-dev libsdl2-image-dev
//g++ testSDL.cpp -lSDL2 -lSDL2_image -o test

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <vector>
//#include <jpeglib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fcntl.h>
#include <cstdio>

using namespace std;

class MulticastSocket {
private:
    int sockfd;
    int port;
    struct ip_mreqn group;
    void init(std::string address) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("Socket creation failed");
            return;
        }

        struct sockaddr_in group_addr;
        memset(&group_addr, 0, sizeof(group_addr));
        group_addr.sin_family = AF_INET;
        group_addr.sin_addr.s_addr = inet_addr(address.c_str());
        group_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&group_addr, sizeof(group_addr)) < 0) {
            perror("Bind failed");
            close(sockfd);
            return;
        }

        group.imr_multiaddr.s_addr = inet_addr(address.c_str());
        group.imr_address.s_addr = INADDR_ANY;
        group.imr_ifindex = 0;  // Default network interface

        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
            perror("Setting multicast group membership failed");
            close(sockfd);
            return;
        }
        //std::cout<<"Multi cast ok\n";
    }
public:
    MulticastSocket(std::string address, int port) {
        this->port=port;
        init(address);
    }
    void setAddress(std::string address) {
        init(address);
    }
    char* receive(int *size) {
        char buffer[1<<20];
        ssize_t len = recvfrom(sockfd, buffer, 1<<20, 0, NULL, NULL);
        if (len < 0) {
            perror("Receive failed");
            close(sockfd);
            return NULL;
        }
        char* data = (char*)malloc(len);
        std::memcpy(data,buffer,len);
        *size=len;
        return data;
    }

    char* nonBlockingReceive(int *size) {
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl failed");
            return NULL;
        }
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("Failed to set non-blocking mode");
            return NULL;
        }
        char buffer[1 << 20];
        ssize_t len = recvfrom(sockfd, buffer, 1 << 20, 0, NULL, NULL);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return NULL;
            } else {
                perror("Receive failed");
                close(sockfd);
                return NULL;
            }
        }
        char* data = (char*)malloc(len);
        std::memcpy(data, buffer, len);
        *size = len;
        return data;
    }

    void closeSocket() {
        // Leave the multicast group and close the socket
        setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group, sizeof(group));
        close(sockfd);
    }
};


void printPixelRGB(SDL_Surface *surface) {
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }
    std::cout<<"Pitch "<<surface->pitch<<std::endl;
    if (surface) {
        SDL_PixelFormat* format = surface->format;
        std::cout << "Pixel format: " << SDL_GetPixelFormatName(format->format) << std::endl;
        std::cout << "Bits per pixel: " << (int)format->BitsPerPixel << std::endl;
        std::cout << "Rmask: " << std::hex << format->Rmask << std::dec << std::endl;
        std::cout << "Gmask: " << std::hex << format->Gmask << std::dec << std::endl;
        std::cout << "Bmask: " << std::hex << format->Bmask << std::dec << std::endl;
    }
    Uint32 *pixels = (Uint32*)surface->pixels;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint32 pixel = pixels[y * surface->w + x];
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            printf("Pixel (%d, %d) - R: %d, G: %d, B: %d\n", x, y, r, g, b);
        }
    }
    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}

SDL_Surface* decode_jpeg_to_surface(const unsigned char *jpeg_data, size_t jpeg_size) {
    int width, height, channels;
    unsigned char *image_data = stbi_load_from_memory(jpeg_data, jpeg_size, &width, &height, &channels, 0);

    if (!image_data) {
        fprintf(stderr, "Failed to decode JPEG image with stb_image\n");
        return NULL;
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_ABGR8888);
    if (surface == nullptr) {
        std::cerr << "Surface could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return NULL;
    }

    SDL_LockSurface(surface);
    //memcpy(surface->pixels, image_data, width * height * channels);
    unsigned char* pixels = (unsigned char*)surface -> pixels;
    for(int y=0;y<surface -> h;y++)
        for(int x=0;x<surface->w;x++) {
            for(int c=0;c<3;c++) {
                pixels[4 * (y * surface -> w + x) + c] = image_data[3 * (y * surface -> w + x) + c]; 
            }
            pixels[4 * (y * surface -> w + x) + 3] = 255;
    }
    SDL_UnlockSurface(surface);

    stbi_image_free(image_data);
    return surface;
}

class Visualizer {
private:
    MulticastSocket *ms;
    bool doContinue;
    SDL_mutex * mutex;
public:
    SDL_Surface* imgSurface;
    SDL_Rect rect;
    int w;
    int h;
    Visualizer(MulticastSocket *ms) {
        this->ms=ms;
        imgSurface=NULL;
        doContinue=true;
        mutex = SDL_CreateMutex();
    }
    void updateSurface(char *data, int size) {
        lock();
        if(imgSurface!=NULL) {
            SDL_FreeSurface(imgSurface);
        }
        if(false) {
            int width=100,height=100;
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_RGBA8888);
            if (surface == nullptr) {
                std::cerr << "Surface could not be created! SDL_Error: " << SDL_GetError() << std::endl;
                SDL_Quit();
                return;
            }
            SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));  // White
            imgSurface = surface;
        } else {
            imgSurface=decode_jpeg_to_surface((const unsigned char*)data, size);
        }
        unlock();
    }
    void receiveAndDisplay() {
        int n;
        //char *data = ms->receive(&n);
        char *data = ms->nonBlockingReceive(&n);
        if(data==NULL)
            return;
        int w,h,x,y,nh,nw,ps;
        memcpy(&w, data, sizeof(int));
        if(w<0) {
            char str[n-3];
            memcpy(&str, data+4, n-4);
            str[n-4]='\0';
            std::cout<<"Received string:\n\n"<<str<<"\n\n";
            return;
        }
        memcpy(&h, data+4, sizeof(int));
        memcpy(&x, data+8, sizeof(int));
        memcpy(&y, data+12, sizeof(int));
        memcpy(&nh, data+16, sizeof(int));
        memcpy(&nw, data+20, sizeof(int));
        memcpy(&ps, data+24, sizeof(int));
        //std::cout<<"w:"<<w<<", h: "<<h<<", x:"<<x<<", y:"<<y<<", nh:"<<nh<<", nw:"<<nw<<", ps:"<<ps<<" ___ n:"<<n<<"\n";
        this->w=w;
        this->h=h;
        int sx=w/nw;
        int sy=h/nh;
        rect.x=x*sx;
        rect.y=y*sy;
        rect.w=sx;
        rect.h=sy;

        updateSurface(data+28,ps);

        free(data);
    }
    void quit() {
        doContinue=false;
    }
    bool isRunning() {
        return doContinue;
    }
    void lock() {
        SDL_LockMutex(mutex);
    }
    void unlock() {
        SDL_UnlockMutex(mutex);
    }
};

int listenToSocketThread(void *pp) {
    Visualizer *visualizer = (Visualizer*)pp;
    while(visualizer->isRunning())
    {
        visualizer->receiveAndDisplay();
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    const char *streams[4] = {
        "225.1.1.1",
        "225.1.1.2",
        "225.1.1.3",
        "225.1.1.4"
    };
    int streamIndex=0;
    if(argc>1) {
        streamIndex = atoi(argv[1]);
        if(streamIndex<0 || streamIndex>3)
            streamIndex=0;
    }    

    SDL_Window *window = SDL_CreateWindow("Schermo del docente", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          640, 480, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event e;

    Visualizer visualizer(new MulticastSocket(streams[streamIndex],5007));

    SDL_Thread* thread = NULL;
    thread= SDL_CreateThread(listenToSocketThread, "Listen To Socket Thread", (void*)&visualizer);
    /*
    std::ifstream file("a.jpg", std::ios::binary);
    std::vector<uint8_t> jpegData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (jpegData.empty()) {
        std::cerr << "Failed to load JPEG file" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }*/

    // Main loop
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        if(visualizer.imgSurface!=NULL) 
        {
            visualizer.lock();
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, visualizer.imgSurface);
            if (texture == NULL) {
                printf("Unable to create texture from surface: %s\n", SDL_GetError());
                return 0;
            }
            //SDL_FreeSurface(surface);
            //SDL_Rect dstRect = {x:0, y:0, w:visualizer.imgSurface->w, h: visualizer.imgSurface->h};
            SDL_RenderCopy(renderer, texture, NULL,&visualizer.rect);// &dstRect);
            visualizer.unlock();
            SDL_DestroyTexture(texture);
        }

        SDL_RenderPresent(renderer);
    }
    visualizer.quit();

    if(thread!=NULL) {
        SDL_WaitThread(thread, NULL);
    }

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
