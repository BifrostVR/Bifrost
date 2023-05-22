// Compile to exe: x86_64-w64-mingw32-g++ -static BifrostVR.cpp deps/ggml.o deps/whisper.o deps/sqlite3.o -o BifrostVR.exe -L./libs/ -lopenvr_api -lSDL2 -lmingw32 -lopengl32 -lgdi32 -luser32 -lkernel32 -limm32 -lversion -lwinmm -lsetupapi -loleaut32 -lole32

#define CNFG_IMPLEMENTATION
#define CNFGOGL
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "deps/rawdraw_sf.h"
#include "deps/stb_image.h"
#include "deps/openvr.h"
#include "deps/whisper.h"
#include "deps/sqlite3.h"
#include "audio_async.h"

using namespace::vr; // "openvr.h" uses the vr namespace, and with openvr making the majority of the cript, it's namespace is preferred

// Rawdraw defs
void HandleKey( int keycode, int bDown ) { }
void HandleButton( int x, int y, int button, int bDown ) { }
void HandleMotion( int x, int y, int mask ) { }
void HandleDestroy() { }
unsigned char * imgPost;

// VR defs
VROverlayHandle_t ulHandle; // OpenVR application ID
TrackedDeviceIndex_t leftHandID; // ID of left remote
TrackedDeviceIndex_t rightHandID; // ID of right remote

// SQL defs
const char * server = "//169.254.136.180/bifrost_texts/posts.sqlite"; // Written here for easier updating
sqlite3 * db;
sqlite3_stmt * stmt;
std::vector<std::string> posts; // Saved posts
int max = -10; // Trakcs the maximum position in posts
int cur = -10; // Tracks the current position in posts
bool notif = true;
char* err;
bool con; // Checks server connectivity

int WIDTH = 256;
int HEIGHT = 256;

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

void UpdateFeed() {
    sqlite3_open(server, &db);
    int check = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Posts(Id INT PRIMARY KEY AUTOINCREMENT, PostTextArea TEXT, FilePath TEXT);", NULL, NULL, &err);
    con = check == SQLITE_OK;
    sqlite3_prepare_v2(db, "SELECT * FROM Posts", -1, &stmt, 0);
    posts.clear();
    while(sqlite3_step(stmt) == SQLITE_ROW) posts.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); // Pushes to posts
    sqlite3_close(db);
    
    if (max != posts.size() - 1) { // Handles altering passive and active uses of the feed
        if (max < posts.size() - 1) notif = true;
        if (cur == max) cur = posts.size() - 1;
        max = posts.size() - 1;
        if (cur > max) cur = max;
        if (cur == -1) cur = max;
    }
}

std::string Segment(std::string seperate) {
    std::string newOne = seperate;
    if (newOne.length() > 28) {
        int toLoop = newOne.length() / 28;
        for (int j = 0; j < toLoop; j++) {
            for (int i = std::min(28 * (j + 1), (int)newOne.length() - 1); i < 0; i--) {
                if (newOne[i] == ' ') {
                    newOne[i] = '\n';
                    break;
                }
            }
        }
    }
    return newOne;
}

int main()
{
    CNFGSetup( "BifrostVR", -WIDTH, -HEIGHT); // Negative hides the application window on desktop, leaving only the terminal
    int iw, ih, ic; // Image deps
    imgPost = stbi_load("Resources/post.png", &iw, &ih, &ic, 0);
    GLuint texture = 0; // Prepare OpenGL texture for future binding
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }

    
    EVRInitError e;
    VR_Init(&e, VRApplication_Overlay); // Sets application type as overlay
    if (e != VRInitError_None) return 1; // Stops running if SteamVR fails launch
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
    std::vector<float> pcmf32; // HOlds post audio for processing

    // Event bools
    bool open = false; // Open beyond the icon
    bool closed = true; // Closed to icon
    bool down = false; // Keeps down scrolling controllable
    bool post = false; // In the post menu
    bool recording = false; // Actively recording
    bool msg = false; // Message made
    bool aFail = false; // Handles audio failure
    bool dcrfix = false; // "Right click" triggers on dpad down AND up, this fixes that
    bool dclfix = false; // "Left click" triggers on trigger pull AND release, this fixes that
    
    // Misc
    bool hands = false;
    std::string content; 
    time_t ct1, ct2; // Closing timers
    time_t pt1, pt2; // Post pull timers;
    time(&pt1);
    while(CNFGHandleInput())
    {
        time(&pt2);
        if (difftime(pt2, pt1) > 3) { // Puts a limit on how fast UpdateFeed is called to prevent corruprtion
            time(&pt1);
            UpdateFeed(); // Updates local SQL vector
        }

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
                        time(&ct1);
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
                            aFail = false;
                        }
                    }
                    else if (cEvent.data.mouse.button == VRMouseButton_Right && dcrfix) dcrfix = false;
                    if (!cEvent.data.scroll.unused && !down) {
                        if (cEvent.data.scroll.ydelta < -0.9) {
                            down = true;
                            if (cur != 0) cur--;
                        }
                    }
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
                            if (msg) {
                                content = "";
                                whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()); // Pushes all audio data to whisper
                                const int n_segments = whisper_full_n_segments(ctx);
                                for (int i = 0; i < n_segments; ++i) content += whisper_full_get_segment_text(ctx, i); // Converts listed words to string
                            }
                            else {
                                content = "Audio failed";
                                msg = true;
                                aFail = true;
                            }
                        }
                        else if (!recording && msg && !aFail) {
                            sqlite3_open(server, &db); // Pushes from whisper to server
                            std::string query = "INSERT INTO Posts (PostTextArea, FilePath) VALUES ('" + content + "', NULL);";
                            sqlite3_exec(db, query.c_str(), NULL, NULL, &err); // Executes query
                            sqlite3_close(db);
                            post = false;
                            msg = false;
                        }
                    }
                    else if (cEvent.data.mouse.button == VRMouseButton_Left && !post && !dclfix) {
                        dclfix = true;
                        if (cur != max) cur++;
                    }
                    else if (cEvent.data.mouse.button == VRMouseButton_Left && dclfix) dclfix = false;
                    time(&ct1);
                }
                time(&ct2);
                down = false;
                if (difftime(ct2, ct1) > 5 && !post) {
                    open = false;
                    closed = true;
                }
                notif = false;
            }

            // This if else tree turns the interactions into texture changes on the overlay
            if (open) {
                if (!post) {
                    if (!con) CNFGDrawText("Failed to fetch server", 3);
                    else {
                        if (cur < 0) CNFGDrawText("Not much here...", 3);
                        else CNFGDrawText(Segment(posts[cur]).c_str(), 3 );
                        CNFGBlitImage((unsigned int*)imgPost, 10, 120, iw, ih);
                        CNFGColor( 0x5CE1E6ff);
                        CNFGPenX = 48; CNFGPenY = 130;
                        CNFGDrawText( "Post something", 3 );
                    }
                }
                else {
                    if (!recording && !msg) CNFGDrawText("Pull trigger to start message", 3);
                    else if (!recording && msg) CNFGDrawText(Segment(content).c_str(), 3); // Uses the raw string because it hasn't been posted yet
                    else CNFGDrawText("Pull trigger to end message", 3);
                    if (!recording) {
                        CNFGBlitImage((unsigned int*)imgPost, 10, 120, iw, ih);
                        CNFGColor( 0x5CE1E6ff);
                        CNFGPenX = 48; CNFGPenY = 130;
                        CNFGDrawText( "Back to board", 3 );
                    }
                }
            }
            else if (closed) {
                if (notif) {
                    CNFGColor( 0x5CE1E6ff);
                    RDPoint points[8] = { { 112, 130 }, { 112, 114 }, { 120, 106 }, { 136, 106 }, {144, 114}, {144, 130}, {136, 138}, {120, 138}};
                    CNFGTackPoly( points, 8 );
                }
                CNFGColor( 0x3f3f3fff );
                RDPoint points[8] = { { 118, 127 }, { 118, 117 }, { 123, 112 }, { 133, 112 }, {138, 117}, {138, 127}, {133, 132}, {123, 132}};
                CNFGTackPoly( points, 8 );
                CNFGPenX = 120; CNFGPenY = 115; // Overides standard (1,1)
                if (notif) CNFGColor( 0x5CE1E6ff);
                else CNFGColor( 0xffffffff );
                CNFGDrawText( "BF", 3 );
            }
        }
        RawToVR(texture);
    }
    
    whisper_free(ctx); // Clears whisper for later reuse
}