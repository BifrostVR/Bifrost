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
TrackedDeviceIndex_t leftHandID; // ID of left remote
TrackedDeviceIndex_t rightHandID; // ID of right remote

// Bifrost specific val defs
int WIDTH = 256;
int HEIGHT = 32;

// Bifrost original functions
bool BindHand()
{
    if (leftHandID != 0 && leftHandID != -1) return true;
    leftHandID = VRSystem()->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand);
    if (leftHandID == 0 || leftHandID == -1) return false;
    printf("Left controller ID gathered\n");
    HmdMatrix34_t transform = { 0 };
    transform.m[0][0] = 1;
    transform.m[1][3] = .2;
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
    VROverlay()->SetOverlayWidthInMeters(ulHandle, 0.3);
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
    bool rc;
    bool lc;
    bool down;
    bool up;
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
            while(VROverlay()->PollNextOverlayEvent( ulHandle, &cEvent, sizeof(cEvent))) // Process events
            {
                if (cEvent.data.mouse.button == VRMouseButton_Left) {
                    lc = true;
                    rc = false;
                    down = false;
                    up = false;
                }
                else if (!cEvent.data.scroll.unused) {
                    if (cEvent.data.scroll.ydelta < -0.9) {
                        down = true;
                        lc = false;
                        rc = false;
                        up = false;
                    }
                    else if (cEvent.data.scroll.ydelta > 0.9) {
                        up = true;
                        lc = false;
                        rc = false;
                        down = false;
                    }
                }
                else if (cEvent.data.mouse.button == VRMouseButton_Right) { // checked last because right is on the scroll wheel
                    rc = true;
                    lc = false;
                    down = false;
                    up = false;
                }
            }
            if (rc) CNFGDrawText( "Right", 4 );
            else if (lc) CNFGDrawText( "Left", 4 );
            else if (down) CNFGDrawText( "Down", 4 );
            else if (up) CNFGDrawText( "Up", 4 );
            else CNFGDrawText( "No trigger yet", 4 );
        }

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
}