// Compile to exe: x86_64-w64-mingw32-g++ BifrostVR.cpp -o test.exe -lopengl32 -lgdi32 -luser32 -lkernel32 -L./libs/ -lopenvr_api
// The "openvr_api.dll" file must be present in the location of output exe to run it

#define CNFG_IMPLEMENTATION
#define CNFGOGL
#include "rawdraw_sf.h"
#include "openvr.h"
#include <stdio.h>
#include <time.h>

using namespace::vr; // "openvr.h" uses the vr namespace, so cout and cin will not be available in this specific script

// Rawdraw function defs
void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

VROverlayHandle_t ulHandle; // OpenVR application ID
TrackedDeviceIndex_t leftHandID; // ID of left remote
TrackedDeviceIndex_t rightHandID; // ID of right remote

// OpenGL info
int WIDTH = 256;
int HEIGHT = 64;

FILE * sFile; // Server info

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

    bool hands = false;
    bool lc = false;;
    bool closed = true;;
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
            // This if else tree is the meat of the program. It checks for inactivty and processes inputs for interaction.
            if (closed) {
                while(VROverlay()->PollNextOverlayEvent( ulHandle, &cEvent, sizeof(cEvent))) // Process events
                {
                    if (cEvent.data.mouse.button == VRMouseButton_Left) {
                        lc = true;
                        closed = false;
                        time(&time1);
                    }
                }
            }
            else {
                while(VROverlay()->PollNextOverlayEvent( ulHandle, &cEvent, sizeof(cEvent))) // Process events
                {
                    time(&time1);
                }
                time(&time2);
                if (difftime(time2, time1) > 5) {
                    lc = false;
                    closed = true;
                }
            }

            // This if else tree turns the interactions into texture changes on the overlay
            if (lc) {
                bool con = true;
                sFile = fopen( "//169.254.136.180/bifrost_texts/Test.txt", "r"); // Connects to the servers output
                if (sFile == NULL) {
                    CNFGDrawText("Failed to fetch server", 4);
                    con = false;
                }
                if (con) {
                    char contents[100];
                    fgets(contents, 100, sFile);
                    CNFGPenX = 1; CNFGPenY = 1;
                    CNFGDrawText( contents, 4 );
                }
            }
            else if (closed) {
                CNFGColor( 0x3f3f3fff );
                RDPoint points[8] = { { 118, 27 }, { 118, 17 }, { 123, 12 }, { 133, 12 }, {138, 17}, {138, 27}, {133, 32}, {123, 32}};
                CNFGTackPoly( points, 8 );
                CNFGPenX = 120; CNFGPenY = 15;
                CNFGColor( 0xffffffff );
                CNFGDrawText( "BF", 3 );
            }
        }

        CNFGFlushRender(); // Finalizes all rawdraw changes within this frame
        if (sFile != NULL) fclose(sFile); // Disconects after finilization of text

        glBindTexture(GL_TEXTURE_2D, texture); // Grabs current OpenGL window and binds it to texture
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, WIDTH, HEIGHT, 0);
        struct Texture_t tex; // Converts OpenGL texture to OpenVR texture
        tex.eColorSpace = ColorSpace_Auto;
        tex.eType = TextureType_OpenGL;
        tex.handle = (void*)(intptr_t)texture;

        VROverlay()->SetOverlayTexture( ulHandle, &tex ); // Send texture into OpenVR
       
        CNFGSwapBuffers(); // Prepares rawdraw for next frame 
    }
}