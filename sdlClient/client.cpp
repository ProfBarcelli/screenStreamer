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
    void closeSocket() {
        // Leave the multicast group and close the socket
        setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group, sizeof(group));
        close(sockfd);
    }
};


void printPixelRGB(SDL_Surface *surface) {
    // Lock the surface to access pixel data if needed (only if SDL surface needs to be locked)
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

    // Get the pointer to the pixels
    Uint32 *pixels = (Uint32*)surface->pixels;

    // Iterate through each pixel
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            // Get the pixel's value at (x, y)
            Uint32 pixel = pixels[y * surface->w + x];
            //std::cout<<pixel<<std::endl;

            // Extract RGB components using SDL_GetRGB
            Uint8 r, g, b;
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            // Print the RGB values
            printf("Pixel (%d, %d) - R: %d, G: %d, B: %d\n", x, y, r, g, b);
        }
    }

    // Unlock the surface if it was locked
    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}

SDL_Surface* decode_jpeg_to_surface(const unsigned char *jpeg_data, size_t jpeg_size) {
    // Step 1: Decode the JPEG data with stb_image
    int width, height, channels;
    unsigned char *image_data = stbi_load_from_memory(jpeg_data, jpeg_size, &width, &height, &channels, 0);
/*
    for(int i=0;i<width * height * channels;i++)
        std::cout<<i<<"->"<<(int)image_data[i]<<std::endl;

    std::cout<<"w: "<<width<<", h: "<<height<<", c: "<<channels<<"\n";
*/
    if (!image_data) {
        fprintf(stderr, "Failed to decode JPEG image with stb_image\n");
        return NULL;
    }

    // Step 2: Convert image to an SDL_Surface
    // If the image has 3 channels (RGB), we can use SDL_PIXELFORMAT_RGB888.
    // If the image has 4 channels (RGBA), we can use SDL_PIXELFORMAT_RGBA8888.
    /*int pitch = width * 3;
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(image_data, width, height,
        24,                  // Bits per pixel (24 for RGB)
        pitch,               // Pitch (row size in bytes, excluding padding)
        0x0000FF,            // Red mask (0x0000FF = 8 bits for blue)
        0x00FF00,            // Green mask (0x00FF00 = 8 bits for green)
        0xFF0000,            // Blue mask (0xFF0000 = 8 bits for red)
        0                    // No alpha channel (RGB, not RGBA)
    );*/
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_ABGR8888);
    if (surface == nullptr) {
        std::cerr << "Surface could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return NULL;
    }


    //SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));  // White


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

    //printPixelRGB(surface);
    // Step 4: Free the image data (stbi_load_from_memory copies the data into our surface)
    stbi_image_free(image_data);

    return surface;
}

class Visualizer {
private:
    MulticastSocket *ms;
    SDL_Renderer *renderer;
    bool doContinue;
public:
    //SDL_Texture* texture;
    SDL_Surface* imgSurface;
    Visualizer(MulticastSocket *ms, SDL_Renderer *renderer) {
        this->ms=ms;
        this->renderer=renderer;
        imgSurface=NULL;
        doContinue=true;
    }
    void updateSurface(char *data, int size) {

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
        /*
        if(texture!=NULL) {
            free(texture);
        }

        // Create a texture from the surface
        texture = SDL_CreateTextureFromSurface(renderer, imgSurface);
        SDL_FreeSurface(imgSurface);  // Free the surface after texture is created

        if (!texture) {
            std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << std::endl;
            return;
        }

        // Get the width and height of the texture
        //int width, height;
        //SDL_QueryTexture(texture, NULL, NULL, &width, &height);

        // Print out the dimensions of the texture
        //std::cout << "Texture width: " << width << ", height: " << height << std::endl;
        */
    }
    void receiveAndDisplay() {
        std::cout<<"awating data\n";
        int n;
        char *data = ms->receive(&n);
        std::cout<<"reveived "<<n<<" bytes\n";

        int w,h,x,y,nh,nw,ps;
        memcpy(&w, data, sizeof(int));
        if(w<0) {
            char str[n-3];
            memcpy(&str, data+4, n-4);
            str[n-4]='\0';
            std::cout<<"Received string: "<<str<<"\n";
            return;
        }
        memcpy(&h, data+4, sizeof(int));
        memcpy(&x, data+8, sizeof(int));
        memcpy(&y, data+12, sizeof(int));
        memcpy(&nh, data+16, sizeof(int));
        memcpy(&nw, data+20, sizeof(int));
        memcpy(&ps, data+24, sizeof(int));
        std::cout<<"w:"<<w<<", h: "<<h<<", x:"<<x<<", y:"<<y<<", nh:"<<nh<<", nw:"<<nw<<", ps:"<<ps<<" ___ n:"<<n<<"\n";

        updateSurface(data+28,ps);

        free(data);
    }
    void quit() {
        doContinue=false;
    }
    bool isRunning() {
        return doContinue;
    }
};

int listenToSocketThread(void *pp) {
    Visualizer *visualizer = (Visualizer*)pp;
    std::cout<<"Listening started\n";
    while(visualizer->isRunning())
    {
        visualizer->receiveAndDisplay();
        std::cout<<"Video update\n";
    }
    /*
    for(int i=0;i<10;i++) {
        cout<<"hello "<<i<<"\n";
        usleep(1e6);
    }*/
    return 0;
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Schermo del docente", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          640, 480, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop flag
    int quit = 0;
    SDL_Event e;

    Visualizer visualizer(new MulticastSocket("225.1.1.1",5007), renderer);

    SDL_Thread* thread = NULL;
    thread= SDL_CreateThread(listenToSocketThread, "Listen To Socket Thread", (void*)&visualizer);

   // Load JPEG file into memory (replace with actual file or binary data)
    std::ifstream file("a.jpg", std::ios::binary);
    std::vector<uint8_t> jpegData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (jpegData.empty()) {
        std::cerr << "Failed to load JPEG file" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    //visualizer.updateTexture((char*)jpegData.data(), jpegData.size());

    //SDL_RenderClear(renderer);
    /*
    int width=100,height=100;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_RGBA8888);
    if (surface == nullptr) {
        std::cerr << "Surface could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return NULL;
    }
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));  // White
    visualizer.imgSurface = surface;
    */

    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        if(visualizer.imgSurface!=NULL) 
        {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, visualizer.imgSurface);
            if (texture == NULL) {
                printf("Unable to create texture from surface: %s\n", SDL_GetError());
                return 0;
            }
            //SDL_FreeSurface(surface);
            SDL_Rect dstRect = {x:0, y:0, w:visualizer.imgSurface->w, h: visualizer.imgSurface->h};
            SDL_RenderCopy(renderer, texture, NULL, &dstRect);
            SDL_DestroyTexture(texture);
        }

        // Draw the window contents
        SDL_RenderPresent(renderer);
    }
    visualizer.quit();

    // Wait for the thread to finish
    if(thread!=NULL)
        SDL_WaitThread(thread, NULL);

    // Cleanup and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
