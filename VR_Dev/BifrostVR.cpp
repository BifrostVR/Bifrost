// Compile to exe: x86_64-w64-mingw32-g++ -static BifrostVR.cpp deps/ggml.o deps/whisper.o -o test.exe -L./libs/ -lopenvr_api -lSDL2 -lmingw32 -lopengl32 -lgdi32 -luser32 -lkernel32 -limm32 -lversion -lwinmm -lsetupapi -loleaut32 -lole32
// The "openvr_api.dll" file must be present in the location of output exe to run it

#define CNFG_IMPLEMENTATION
#define CNFGOGL
#include "deps/rawdraw_sf.h"
#include "deps/openvr.h"
#include "deps/whisper.h"
#include "audio_async.h"
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <sqlite3.h>

using namespace::vr; // "openvr.h" uses the vr namespace, and with openvr making the majority of the cript, it's namespace is preferred

// Rawdraw function defs
void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

VROverlayHandle_t ulHandle; // OpenVR application ID
TrackedDeviceIndex_t leftHandID; // ID of left remote
TrackedDeviceIndex_t rightHandID; // ID of right remote

std::fstream sFile; // Server info

int WIDTH = 256;
int HEIGHT = 64;

// Bifrost original functions
bool BindHand()
{
    if (leftHandID != 0 && leftHandID != -1) return true;
    leftHandID = VRSystem()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand);
    if (leftHandID == 0 || leftHandID == -1) return false;
    printf("Left controller ID gathered\n");
    HmdMatrix34_t transform = { 0 };
    transform.m[0][0] = 1;
    transform.m[1][3] = .1;
    transform.m[1][1] = 1;
    transform.m[2][2] = 1;

    VROverlay()->SetOverlayTransformTrackedDeviceRelative(ulHandle, leftHandID, &transform);
    return true;
}

bool FindRight() 
{
    if (rightHandID != 0 && rightHandID != -1) return true;
    rightHandID = VRSystem()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_RightHand);
    if (rightHandID == 0 || rightHandID == -1) return false;
    printf("Right controller ID gathered\n");
    return true;
}

void RawToVR(GLuint texture) {
    CNFGFlushRender(); // Finalizes all rawdraw changes within this frame

    glBindTexture(GL_TEXTURE_2D, texture); // Grabs current OpenGL window and binds it to texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, WIDTH, HEIGHT, 0);
    struct Texture_t tex; // Converts OpenGL texture to OpenVR texture
    tex.eColorSpace = ColorSpace_Auto;
    tex.eType = TextureType_OpenGL;
    tex.handle = (void*)(intptr_t)texture;

    VROverlay()->SetOverlayTexture( ulHandle, &tex ); // Send texture into OpenVR
       
    CNFGSwapBuffers(); // Prepares rawdraw for next frame 
}

int main()
{
    CNFGSetup( "BifrostVR", -WIDTH, -HEIGHT); // Negative hides the application window on desktop, leaving only the terminal
    {
        EVRInitError e;
        VR_Init(&e, VRApplication_Overlay); // Sets application type as overlay
        if (e != VRInitError_None) return 1;
    }

    VROverlay()->CreateOverlay("BifrostVR-tag", "BifrostVR", &ulHandle); // Prepares and starts overlay
    VROverlay()->SetOverlayWidthInMeters(ulHandle, 0.25);
    VROverlay()->SetOverlayColor(ulHandle, 1,1,1);
    VROverlay()->SetOverlayInputMethod(ulHandle, VROverlayInputMethod_Mouse); // Enables VR controller interactions
    VROverlay()->SetOverlayFlag(ulHandle, VROverlayFlags_MakeOverlaysInteractiveIfVisible, true);
    VROverlay()->SetOverlayFlag(ulHandle, VROverlayFlags_SendVRTouchpadEvents, true);
    VROverlay()->SetOverlayFlag(ulHandle, VROverlayFlags_SendVRDiscreteScrollEvents, true);
    VRTextureBounds_t bounds; // Prepares overlay for future texture handling
    bounds.uMin = 0;
    bounds.uMax = 1;
    bounds.vMin = 0;
    bounds.vMax = 1;
    VROverlay()->SetOverlayTextureBounds(ulHandle, &bounds);
    VROverlay()->ShowOverlay(ulHandle);
    
    GLuint texture = 0; // Prepare OpenGL texture for future binding
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }

    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY); // Properties needed to use whisper.h
    {
        wparams.print_progress   = false;
        wparams.print_special    = false;
        wparams.print_realtime   = false;
        wparams.print_timestamps = false;
        wparams.translate        = false;
        wparams.single_segment   = false;
        wparams.max_tokens       = 32;
        wparams.language         = "en";
        wparams.n_threads        = std::max(1, std::min(7, (int32_t) std::thread::hardware_concurrency()));
        wparams.audio_ctx        = 768;
        wparams.speed_up         = false;
        wparams.temperature_inc  = wparams.temperature_inc;
        wparams.prompt_tokens    = nullptr;
        wparams.prompt_n_tokens  = 0;
    }
    whisper_context * ctx = whisper_init_from_file("ggml-base.en.bin"); // Engages whisper.h
    audio_async audio; // Prepares for audio capture
    audio.init();
    std::vector<float> pcmf32;

    // Event bools
    bool open = false; // Open beyond the icon
    bool closed = true; // Closed to icon
    bool post = false; // In the post menu
    bool recording = false; // Actively recording
    bool msg = false; // Message made
    bool dcrfix = false; // "Right click" triggers on dpad down AND up, this fixes that
    bool dclfix = false; // "Left click" triggers on trigger pull AND release, this fixes that
    bool hands = false;
    std::string content;
    time_t time1, time2;
    while(CNFGHandleInput())
    {
        CNFGBGColor = 0x00000000; //Transparent background
        CNFGClearFrame();

        CNFGColor( 0xffffffff ); //Set draw color to white
		CNFGPenX = 1; CNFGPenY = 1;

        struct VREvent_t cEvent;
        if (!hands) // Adds left and right hand to memory and locks overlay over left hand
        { 
            bool left = BindHand();
            bool right = FindRight();
            if (left && right) hands = true;
            CNFGDrawText( "Waiting on remotes", 4 );
        }  
        else
        {
            // This if else tree is the meat of the program. It checks for inactivty and processes inputs for interaction and stores them as bools
            if (closed) {
                while(VROverlay()->PollNextOverlayEvent( ulHandle, &cEvent, sizeof(cEvent))) // Process events
                {
                    if (cEvent.data.mouse.button == VRMouseButton_Left) {
                        open = true;
                        closed = false;
                        time(&time1);
                    }
                }
            }
            else {
                while(VROverlay()->PollNextOverlayEvent( ulHandle, &cEvent, sizeof(cEvent))) // Process events
                {
                    if (cEvent.data.mouse.button == VRMouseButton_Right && !dcrfix) {
                        dcrfix = true;
                        if (!recording && !msg) post = !post;
                        else if (!recording && msg) {
                            msg = false;
                        }
                    }
                    else if (cEvent.data.mouse.button == VRMouseButton_Right && dcrfix) dcrfix = false;
                    if (cEvent.data.mouse.button == VRMouseButton_Left && post && !dclfix) {
                        dclfix = true;
                        if (!recording && !msg) {
                            recording = audio.start();
                        }
                        else if (recording && !msg) {
                            recording = false;
                            CNFGDrawText("Processing...", 3);
                            RawToVR(texture);
                            msg = audio.end(pcmf32);
                            msg = true;
                            if (msg) {
                                content = "";
                                whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()); // Pushes all audio data to whisper
                                const int n_segments = whisper_full_n_segments(ctx);
                                for (int i = 0; i < n_segments; ++i) content += whisper_full_get_segment_text(ctx, i); // Converts listed words to string
                                if (content.length() >= 3) if (content.substr(content.length() - 3,3) == "you") content.erase(content.length() - 3, 3); // Fixes silcene error
                            }
                            else {
                                content = "Audio failed";
                                msg = true;
                            }
                        }
                        else if (!recording && msg) {
                            // This is where we will post to server, for now only bools are handled
                            post = false;
                            msg = false;
                        }
                    }
                    else if (cEvent.data.mouse.button == VRMouseButton_Left && post && dclfix) dclfix = false;
                    time(&time1);
                }
                time(&time2);
                if (difftime(time2, time1) > 5 && !post) {
                    open = false;
                    closed = true;
                }
            }

            // This if else tree turns the interactions into texture changes on the overlay
            if (open) {
                if (!post) {
                    bool con = true;
                    sFile.open("//169.254.136.180/bifrost_texts/Test.txt", std::ios::app | std::ios::in); // Connects to the servers output
                    if (!sFile.is_open()) {
                        CNFGDrawText("Failed to fetch server", 3);
                        con = false;
                    }   
                    if (con) {
                        // Will be reformated for final build, current format is for proof of concept only
                        std::string st; // Server texts
                        getline(sFile, st);
                        CNFGDrawText(st.c_str(), 3 );
                    }
                }
                else {
                    if (!recording && !msg) CNFGDrawText("Pull trigger to record message", 3);
                    else if (!recording && msg) CNFGDrawText(content.c_str(), 3); // USes the raw string because it hasn't
                    else CNFGDrawText("Pull trigger to end message", 3);
                }
            }
            else if (closed) {
                CNFGColor( 0x3f3f3fff );
                RDPoint points[8] = { { 118, 27 }, { 118, 17 }, { 123, 12 }, { 133, 12 }, {138, 17}, {138, 27}, {133, 32}, {123, 32}};
                CNFGTackPoly( points, 8 );
                CNFGPenX = 120; CNFGPenY = 15; // Overides standard (1,1)
                CNFGColor( 0xffffffff );
                CNFGDrawText( "BF", 3 );
            }
        }

        if (sFile.is_open()) sFile.close(); // Disconects after finilization of text
        RawToVR(texture);
    }
    
    whisper_free(ctx); // Clears whisper for later reuse
}