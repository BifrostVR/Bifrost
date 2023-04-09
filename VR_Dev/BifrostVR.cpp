// Compile to exe: x86_64-w64-mingw32-g++ BifrostVR.cpp -o test.exe -lopengl32 -lgdi32 -luser32 -lkernel32 -L./libs/ -lopenvr_api
// The "openvr_api.dll" file must be present in the location of output exe to run it

#define CNFG_IMPLEMENTATION
#define CNFGOGL
#include "rawdraw_sf.h"
#include "openvr.h"
#include <stdio.h>

using namespace::vr; // "openvr.h" uses the vr namespace, so cout and cin will not be available in this specific script

// Rawdraw function defs
void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }

VROverlayHandle_t ulHandle; // OpenVR application ID

// Bifrost specific val defs
int WIDTH = 256;
int HEIGHT = 256;

// Bifrost original functions
bool FindHand()
{
    TrackedDeviceIndex_t leftHandID;
    leftHandID = VRSystem()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand);
    if (leftHandID == 0 || leftHandID == -1) return false;
    printf("Controller ID gathered");
    HmdMatrix34_t transform = { 0 };
    transform.m[0][0] = 1;
    transform.m[1][1] = 1;
    transform.m[2][2] = 1;

    VROverlay()->SetOverlayTransformTrackedDeviceRelative(ulHandle, leftHandID, &transform);
    return true;
}

int main()
{
    CNFGSetup( "Hello, world", WIDTH, HEIGHT);
    {
        EVRInitError e;
        VR_Init(&e, VRApplication_Overlay); // Sets application type as overlay
        if (e != VRInitError_None) return 1;
    }

    VROverlay()->CreateOverlay("Hello, world-tag", "Hello, world", &ulHandle); // Prepares and starts overlay
    VROverlay()->SetOverlayWidthInMeters(ulHandle, 0.3);
    VROverlay()->SetOverlayColor(ulHandle, 1,1,1);
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
    while(CNFGHandleInput())
    {
        CNFGBGColor = 0x000000ff; //Transparent background
        CNFGClearFrame();

        CNFGColor( 0xffffffff ); //Set draw color to white
		CNFGPenX = 1; CNFGPenY = 1;
        CNFGDrawText( "Hello, World", 4 );

        CNFGFlushRender(); // Finalizes all rawdraw changes within this frame

        if (!hands) hands = FindHand(); // Attatches drawn window to (left) hand for ease of view

        glBindTexture(GL_TEXTURE_2D, texture); // Grabs current OpenGL window and binds it to texture
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, WIDTH, HEIGHT, 0);
        struct Texture_t tex; // Converts OpenGL texture to OpenVR texture
        tex.eColorSpace = ColorSpace_Auto;
        tex.eType = TextureType_OpenGL;
        tex.handle = (void*)(intptr_t)texture;

        VROverlay()->SetOverlayTexture( ulHandle, &tex ); // Send texture into OpenVR
       
        struct VREvent_t nEvent; // Process through texture events.
        if (hands) VROverlay()->PollNextOverlayEvent( ulHandle, &nEvent, 0xffffff );
       
        CNFGSwapBuffers(); // Prepares rawdraw for next frame 
    }
}