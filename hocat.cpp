/*
 * Copyright 2026, Kris Beazley hocat@epluribusunix.net
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Application.h>
#include <Window.h>
#include <Button.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <TextControl.h>
#include <TextView.h>
#include <ScrollView.h>
#include <LayoutBuilder.h>
#include <String.h>
#include <OS.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <Roster.h>
#include <IconUtils.h>   
#include <Bitmap.h>   
#include <PictureButton.h>
#include <View.h>
#include <NodeInfo.h> 
#include <Alert.h> 
#include <Notification.h>

namespace AppInfo {
    static const char* const VERSION_STRING = "HoCat v1.0.1 (Haiku OS)";
}

bool debugEnable = true;
bool showUpdateNotifications = true;

// Forward declaration signature for update worker thread
static int32 BackgroundUpdateChecker(void* data);

// =============================================================================
// NATIVE ASYNCHRONOUS UPDATE ENGINE IMPLEMENTATION (CURL ENGINE PASS)
// =============================================================================
static int32 BackgroundUpdateChecker(void* data) {
    // Wait a brief 5 seconds after application boot to allow UI rendering to finalize completely
    snooze(5000000); 

    if (debugEnable) printf("[DEBUG_UPDATE] Asynchronous curl update checker running...\n");

    const char* targetUrl = "https://raw.githubusercontent.com/ablyssx74/hocat/refs/heads/main/VERSION";

    BString shellCmdString;
    shellCmdString.SetToFormat("curl -sL \"%s\"", targetUrl);

    BString remoteVersionStr = "";
    
    FILE* pipeStream = popen(shellCmdString.String(), "r");
    if (pipeStream != nullptr) {
        char buffer[128] = {0};
        if (fgets(buffer, sizeof(buffer), pipeStream) != nullptr) {
            remoteVersionStr = buffer;
        }
        pclose(pipeStream);
    }

    remoteVersionStr.Trim(); 
    if (debugEnable) printf("[DEBUG_UPDATE] Raw text received from GitHub: '%s'\n", remoteVersionStr.String());

    remoteVersionStr.Trim(); 
    if (debugEnable) printf("[DEBUG_UPDATE] Raw text received from GitHub: '%s'\n", remoteVersionStr.String());
    
    if (remoteVersionStr.Length() > 0) {
        BString currentVersionStr = AppInfo::VERSION_STRING;
        if (debugEnable) printf("[DEBUG_UPDATE] Local AppInfo text before cleaning: '%s'\n", currentVersionStr.String());

        int32 curMajor = 0, curMinor = 0, curRevision = 0;
        int32 remMajor = 0, remMinor = 0, remRevision = 0;

        // --- Bulletproof sscanf Pattern Matching ---
        // Looks for a 'v' immediately followed by a number, bypassing words like "HaikuDVR" or "Version"
        if (sscanf(currentVersionStr.String(), "%*[^v]v%d.%d.%d", &curMajor, &curMinor, &curRevision) != 3) {
            // Fallback: search for raw dot-separated numbers anywhere if 'v' isn't found
            sscanf(currentVersionStr.String(), "%*[^0-9]%d.%d.%d", &curMajor, &curMinor, &curRevision);
        }

        // Parse the remote string from GitHub using the same pattern rules
        if (sscanf(remoteVersionStr.String(), "%*[^v]v%d.%d.%d", &remMajor, &remMinor, &remRevision) != 3) {
            sscanf(remoteVersionStr.String(), "%*[^0-9]%d.%d.%d", &remMajor, &remMinor, &remRevision);
        }

        // Log the cleaned string results visually just for your debug logs
        if (debugEnable) {
            printf("[DEBUG_UPDATE] Cleaned local target string: '%d.%d.%d'\n", curMajor, curMinor, curRevision);
        }

        // Flatten values down into integers for math checks
        int32 currentFlattened = (curMajor * 10000) + (curMinor * 100) + curRevision;
        int32 remoteFlattened  = (remMajor * 10000) + (remMinor * 100) + remRevision;

        if (debugEnable) {
            printf("[DEBUG_UPDATE] Calculated values for math match -> Local: %d | Remote: %d\n", 
                   (int)currentFlattened, (int)remoteFlattened);
        }


        if (remoteFlattened > currentFlattened) {
            if (debugEnable) printf("[DEBUG_UPDATE] Update matched! Checking alert preference flags...\n");
            
            // =========================================================================
            // CHANNELS AUTO-HIDE PREFERENCE INTERCEPT
            // =========================================================================
            if (!showUpdateNotifications) {
                if (debugEnable) printf("[DEBUG_UPDATE] Suppressing desktop alert toast\n");
                return B_OK; // Break out cleanly and silently without throwing the alert box!
            }
            // =========================================================================

            // Native Haiku desktop notification banner toast window dispatch engine
            BNotification updateAlert(B_INFORMATION_NOTIFICATION);
            updateAlert.SetGroup("HoCat");
            updateAlert.SetTitle("Update Available");
            
            BString alertContent;
            alertContent << "A newer version of HoCat is available! (v" << remoteVersionStr 
                         << ")";
            updateAlert.SetContent(alertContent.String());
            
            updateAlert.Send();
            if (debugEnable) printf("[DEBUG_UPDATE] Toast notification sent successfully.\n");
        } else {
            if (debugEnable) printf("[DEBUG_UPDATE] Math complete: Client binary is already completely up to date.\n");
        }
    } else {
        if (debugEnable) printf("[DEBUG_UPDATE] CRITICAL ERR: Raw text data read from pipe buffer was empty!\n");
    }
    
    return B_OK;
}



const unsigned char kIconHoCat[] = {
	0x6e, 0x63, 0x69, 0x66, 0x08, 0x05, 0x00, 0x05, 0xb8, 0x02, 0x00, 0x16, 0x02, 0x3a, 0x69, 0x2e,
	0x36, 0x69, 0x2f, 0xba, 0x2e, 0xcd, 0x3e, 0x2e, 0xcd, 0x4b, 0x89, 0xa4, 0x49, 0x63, 0x18, 0x00,
	0x5c, 0xff, 0x83, 0x05, 0xe7, 0x02, 0x01, 0x06, 0x02, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3c, 0x00, 0x00, 0x46, 0x80, 0x00, 0x46, 0x80, 0x00, 0x00, 0x29, 0xb6, 0xff, 0xff,
	0x03, 0x54, 0x92, 0x02, 0x01, 0x06, 0x02, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3c, 0x00, 0x00, 0x46, 0x80, 0x00, 0x46, 0x80, 0x00, 0x00, 0x9f, 0xf6, 0x99, 0xff, 0x04, 0x9a,
	0x43, 0x04, 0x01, 0x6e, 0x02, 0x00, 0x06, 0x02, 0x39, 0x59, 0xb6, 0x38, 0x2f, 0x60, 0xba, 0x2f,
	0x60, 0x3b, 0x59, 0xb6, 0x47, 0xc1, 0x42, 0x4b, 0x11, 0xd3, 0x00, 0x56, 0xff, 0x22, 0xff, 0x05,
	0xd0, 0x05, 0x0c, 0x0a, 0x04, 0x4c, 0x60, 0x51, 0x60, 0x60, 0x50, 0x5c, 0x4e, 0x02, 0x04, 0x33,
	0x34, 0xbd, 0x6e, 0x34, 0xb7, 0xb5, 0x34, 0x26, 0x39, 0x26, 0xbb, 0xdc, 0x26, 0xbe, 0x0f, 0x33,
	0x3e, 0xb7, 0xb5, 0x3e, 0xbd, 0x6e, 0x3e, 0x40, 0x39, 0x40, 0xbe, 0x0f, 0x40, 0xbb, 0xdc, 0x0a,
	0x06, 0x51, 0x3a, 0x5a, 0x3d, 0x5a, 0x50, 0x4c, 0x5e, 0x42, 0x58, 0x42, 0x44, 0x0a, 0x04, 0x4c,
	0x48, 0x4c, 0x5e, 0x42, 0x58, 0x42, 0x44, 0x0a, 0x04, 0x51, 0x3a, 0x5a, 0x3d, 0x4c, 0x48, 0x42,
	0x44, 0x0a, 0x04, 0x4c, 0x48, 0x5a, 0x3d, 0x5a, 0x50, 0x4c, 0x5e, 0x04, 0x04, 0xbe, 0x40, 0x4a,
	0x32, 0x46, 0x32, 0x4a, 0x32, 0x3e, 0x42, 0x36, 0x42, 0x3c, 0x42, 0x32, 0x3a, 0x32, 0x02, 0x04,
	0x2e, 0x22, 0xbb, 0x37, 0x22, 0xb5, 0xf0, 0x22, 0x22, 0x2e, 0x22, 0xb5, 0xf0, 0x22, 0xbb, 0x37,
	0x2e, 0x3a, 0xb5, 0xf0, 0x3a, 0xbb, 0x37, 0x3a, 0x3a, 0x2e, 0x3a, 0xbb, 0x37, 0x3a, 0xb5, 0xf0,
	0x06, 0x08, 0xbf, 0xdb, 0xb5, 0x06, 0xb5, 0x60, 0xb5, 0x06, 0xb5, 0x60, 0xb4, 0x43, 0xb6, 0x39,
	0x22, 0x2e, 0x22, 0xb7, 0x58, 0x22, 0xba, 0xb3, 0xb7, 0x18, 0xbd, 0x20, 0xb5, 0x2e, 0xbc, 0x80,
	0xb7, 0x18, 0xbd, 0x20, 0x28, 0x36, 0x28, 0x32, 0x2a, 0x34, 0x26, 0x30, 0x24, 0x30, 0x2e, 0x28,
	0x28, 0x28, 0x2a, 0x28, 0x26, 0x06, 0x0c, 0xa2, 0xeb, 0xaf, 0x2e, 0x28, 0xb9, 0x86, 0x32, 0x26,
	0x34, 0x28, 0x30, 0x2e, 0x30, 0x2c, 0x30, 0x30, 0x36, 0x32, 0x34, 0x36, 0xbb, 0xb3, 0xbc, 0x32,
	0xbb, 0xb3, 0xbc, 0x32, 0xbc, 0xb7, 0xbb, 0x52, 0x3a, 0x2e, 0x3a, 0xba, 0x06, 0x3a, 0xb6, 0x68,
	0xb9, 0xee, 0xb3, 0xfd, 0xbb, 0xe9, 0xb4, 0x93, 0xb9, 0xee, 0xb3, 0xfd, 0x2e, 0x24, 0x30, 0x26,
	0x06, 0x14, 0xbf, 0xdd, 0xaf, 0xab, 0xe8, 0x2e, 0x22, 0xb9, 0x0c, 0x22, 0xb7, 0x2b, 0x22, 0xb5,
	0x06, 0xb5, 0x60, 0xb5, 0xe6, 0xb4, 0x68, 0xb5, 0x06, 0xb5, 0x60, 0x28, 0x28, 0x28, 0x26, 0x28,
	0x2a, 0x24, 0x2e, 0x30, 0x28, 0x32, 0x26, 0x30, 0x2a, 0x34, 0x36, 0xb7, 0x18, 0xbd, 0x20, 0xb7,
	0x18, 0xbd, 0x20, 0xb7, 0x8f, 0xbd, 0x46, 0x2e, 0x3a, 0xb8, 0x0f, 0x3a, 0xb9, 0xc5, 0x3a, 0xbb,
	0xb3, 0xbc, 0x32, 0xba, 0xdc, 0xbc, 0xec, 0xbb, 0xb3, 0xbc, 0x32, 0x34, 0x36, 0x36, 0x32, 0x30,
	0x2e, 0x30, 0x30, 0x30, 0x2c, 0x34, 0x28, 0x32, 0x26, 0xb9, 0x53, 0x28, 0x2e, 0x30, 0x26, 0x2e,
	0x24, 0xb9, 0xee, 0xb3, 0xfd, 0xb9, 0xee, 0xb3, 0xfd, 0xb9, 0x80, 0xb3, 0xdd, 0x02, 0x04, 0x2a,
	0x50, 0xb8, 0x4d, 0x50, 0xb5, 0xaa, 0x50, 0xb4, 0x97, 0x56, 0xb4, 0x97, 0xc7, 0x32, 0xb4, 0x97,
	0xc9, 0xd5, 0x2a, 0x5c, 0xb5, 0xaa, 0x5c, 0xb8, 0x4d, 0x5c, 0x30, 0x56, 0x30, 0xc9, 0xd5, 0x30,
	0xc7, 0x32, 0x09, 0x0a, 0x06, 0x02, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x03, 0x06, 0x02, 0x07, 0x10,
	0x01, 0x17, 0x84, 0x00, 0x04, 0x0a, 0x01, 0x01, 0x03, 0x00, 0x0a, 0x03, 0x01, 0x04, 0x00, 0x0a,
	0x02, 0x01, 0x05, 0x00, 0x0a, 0x04, 0x01, 0x0a, 0x00, 0x0a, 0x05, 0x02, 0x08, 0x09, 0x00, 0x0a,
	0x00, 0x01, 0x0b, 0x10, 0x01, 0x17, 0x88, 0x00, 0x04, 0x0a, 0x07, 0x01, 0x0b, 0x00
};


const size_t kIconHoCatSize = 590;


class IconView : public BView {
public:
    IconView(const char* name) : BView(name, B_WILL_DRAW) {
        fIconBitmap = new BBitmap(BRect(0, 0, 63, 63), B_RGBA32);
        
        // 1. Try to load your custom HVIF array first
        status_t result = BIconUtils::GetVectorIcon(kIconHoCat, sizeof(kIconHoCat), fIconBitmap);
        
        // 2. Fallback: If it's blank or returns an error, pull a sharp native network icon instead
        if (result != B_OK) {
            BMimeType mime("application/x-vnd.Haiku-Network");
            if (mime.GetIcon(fIconBitmap, B_LARGE_ICON) != B_OK) {
                // Ultimate fallback if MIME database is busy: use standard file manager artwork
                BMimeType generic("application/octet-stream");
                generic.GetIcon(fIconBitmap, B_LARGE_ICON);
            }
        }
        
        SetExplicitMinSize(BSize(64, 64));
        SetExplicitMaxSize(BSize(64, 64));
    }

    ~IconView() {
        delete fIconBitmap;
    }

    void Draw(BRect updateRect) override {
        // Match the layout panel color
        SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
        FillRect(Bounds());

        // Alpha transparency enabled
        SetDrawingMode(B_OP_ALPHA);
        DrawBitmap(fIconBitmap, BPoint(0, 0));
    }

private:
    BBitmap* fIconBitmap;
};

// Define application messages
enum {
    MSG_START_SOCAT       = 'strt',
    MSG_STOP_SOCAT        = 'stop',
    MSG_CLEAR_LOG         = 'cler',
    MSG_HELP_SOCAT        = 'help', 
    MSG_ABOUT_HOCAT       = 'abot', 
    MSG_APPEND_TEXT       = 'apnd',
    MSG_THREAD_DONE       = 'done',
    MSG_MENU_CHANGED      = 'menu'
};

class SocatWindow : public BWindow {
public:
        SocatWindow() : BWindow(BRect(100, 100, 750, 600), "HoCat \"Socat\" Controller", 
                            B_DOCUMENT_WINDOW, B_QUIT_ON_WINDOW_CLOSE) {
                            	
                            	
        // =========================================================================
        // AUTOMATED BACKGROUND UPDATE CHECKER THREAD INITIALIZATION
        // =========================================================================
        thread_id updateThread = spawn_thread(BackgroundUpdateChecker, "UpdateCheckerThread", B_NORMAL_PRIORITY, this);
        if (updateThread >= 0) {
            resume_thread(updateThread);
        }
        //
       
        // Build Source PopUp Menu with clean, reliable diagnostic commands
        BPopUpMenu* sourceMenu = new BPopUpMenu("Select Source");
        sourceMenu->AddItem(new BMenuItem("TCP Listener (Port 8080)", new BMessage(MSG_MENU_CHANGED)));
        sourceMenu->AddItem(new BMenuItem("UDP Listener (Port 8080)", new BMessage(MSG_MENU_CHANGED)));
        sourceMenu->AddItem(new BMenuItem("Standard Input (stdin)", new BMessage(MSG_MENU_CHANGED)));
        sourceMenu->AddItem(new BMenuItem("Local File (/boot/home/input.txt)", new BMessage(MSG_MENU_CHANGED)));
        
        sourceMenu->AddSeparatorItem(); 
        sourceMenu->AddItem(new BMenuItem("Test: System Info Generator", new BMessage(MSG_MENU_CHANGED)));
        sourceMenu->AddItem(new BMenuItem("Test: Network Interfaces (ifconfig)", new BMessage(MSG_MENU_CHANGED)));
        sourceMenu->AddItem(new BMenuItem("Test: Routing Table Layout", new BMessage(MSG_MENU_CHANGED)));
		sourceMenu->AddItem(new BMenuItem("Test: HTTP Simple Server (Port 8080)", new BMessage(MSG_MENU_CHANGED)));
        sourceMenu->AddItem(new BMenuItem("Test: HTTPS Secure Server ( Not Supported on WebPositive! )", new BMessage(MSG_MENU_CHANGED)));
        
        sourceMenu->ItemAt(0)->SetMarked(true);
        fSourceField = new BMenuField("source", "Source Preset:", sourceMenu);
        
        // 2. Build Destination PopUp Menu
        BPopUpMenu* destMenu = new BPopUpMenu("Select Destination");
        destMenu->AddItem(new BMenuItem("Standard Output (stdout)", new BMessage(MSG_MENU_CHANGED)));
        destMenu->AddItem(new BMenuItem("Local Log File (/boot/home/socat.log)", new BMessage(MSG_MENU_CHANGED)));
        destMenu->AddItem(new BMenuItem("Connect to Remote TCP", new BMessage(MSG_MENU_CHANGED)));
        destMenu->AddItem(new BMenuItem("Serial Interface (/dev/ports/usb0)", new BMessage(MSG_MENU_CHANGED)));
        destMenu->ItemAt(0)->SetMarked(true);
        fDestField = new BMenuField("dest", "Destination Preset:", destMenu);

        // 3. Fully Editable Raw Arguments Box
        fCommandInput = new BTextControl("cmd", "Arguments:", "-v -v  -d -d tcp-listen:8080,reuseaddr stdout", nullptr);

        // 4. Action Controls
        fStartButton = new BButton("start", "Run Socat", new BMessage(MSG_START_SOCAT));
        fStopButton = new BButton("stop", "Stop", new BMessage(MSG_STOP_SOCAT));
        fStopButton->SetEnabled(false);
        fClearButton = new BButton("clear", "Clear Log", new BMessage(MSG_CLEAR_LOG));
		fHelpButton = new BButton("help", "Help (-h)", new BMessage(MSG_HELP_SOCAT));
		fAboutButton = new BButton("about", "About", new BMessage(MSG_ABOUT_HOCAT));
		
        // Console Output Text View
        fTerminalOutput = new BTextView("output");
        fTerminalOutput->MakeEditable(false);
        fTerminalOutput->SetStylable(true);
        BScrollView* scrollPane = new BScrollView("scroll_output", fTerminalOutput, 0, true, true);

        // Instantiate the live preview frame
        IconView* appIconPreview = new IconView("preview_box");

        // Layout Assembly Grid (Corrected to match your exact variables)
        BLayoutBuilder::Group<>(this, B_VERTICAL, 10)
            .SetInsets(15)
            .AddGrid(5, 5)
                // Position your icon preview spanning multiple rows on the left side (Column 0)
                .Add(appIconPreview, 0, 0, 1, 2)
                
                // Shift your two active presets over to Column 1 and Column 2
                .Add(fSourceField->CreateLabelLayoutItem(), 1, 0)
                .Add(fSourceField->CreateMenuBarLayoutItem(), 2, 0)
                
                .Add(fDestField->CreateLabelLayoutItem(), 1, 1)
                .Add(fDestField->CreateMenuBarLayoutItem(), 2, 1)
                
                // Arguments input box spans cleanly across the remaining grid space
                .Add(fCommandInput->CreateLabelLayoutItem(), 1, 2)
                .Add(fCommandInput->CreateTextViewLayoutItem(), 2, 2)
            .End()
            .AddGroup(B_HORIZONTAL, 10)
                .Add(fStartButton)
                .Add(fStopButton)
                .Add(fClearButton)
                .Add(fHelpButton) 
                .Add(fAboutButton) 
                .AddGlue()
            .End()
            .Add(scrollPane)
        .End();

        
        fThreadId = -1;
        fSocatTeamId = -1;
    }

    void MessageReceived(BMessage* message) override {
        switch (message->what) {


            case MSG_MENU_CHANGED: {
                BMenuItem* srcMarked = fSourceField->Menu()->FindMarked();
                if (srcMarked) {
                    BString label(srcMarked->Label());
                    // If the user selects the secure test line, auto-trigger the generation steps
                    if (label.FindFirst("Secure \"HTTPS\" Web Server Test") != B_ERROR || 
                        label.FindFirst("HTTPS Secure Server") != B_ERROR) {
                        CheckAndGenerateCert();
                    }
                }
                UpdateCommandStringFromPresets();
                break;
            }

            
            case MSG_START_SOCAT: {
                fTerminalOutput->SetText("");
                
                BMenuItem* srcMarked = fSourceField->Menu()->FindMarked();
                BString urlString = "http://127.0.0.1:8080"; // Default address string

                if (srcMarked) {
                    BString labelStr(srcMarked->Label());
                    
                    if (labelStr.FindFirst("HTTP") != B_ERROR || labelStr.FindFirst("HTTPS") != B_ERROR) {
                        fTerminalOutput->Insert("--> HoCat Web Server Started Natively!\n");
                        
                        if (labelStr.FindFirst("HTTPS") != B_ERROR) {
                            urlString = "https://127.0.0.1";
                        }
                        
                        int32 startPos = fTerminalOutput->TextLength();
                        fTerminalOutput->Insert(BString().SetToFormat("Opening web browser to: %s\n\n", urlString.String()).String());
                        int32 endPos = fTerminalOutput->TextLength();
                        
                        text_run_array runArray;
                        runArray.count = 1;
                        runArray.runs[0].offset = startPos;
                        runArray.runs[0].font = be_plain_font;
                        runArray.runs[0].color = {0, 102, 204, 255}; 
                        
                        fTerminalOutput->SetRunArray(startPos, endPos, &runArray);
                    }
                }

                // 1. LOCK CONTROLS
                fStartButton->SetEnabled(false);
                fSourceField->SetEnabled(false);
                fDestField->SetEnabled(false);
                fCommandInput->SetEnabled(false);
                fClearButton->SetEnabled(false);
                fHelpButton->SetEnabled(false); 
                fStopButton->SetEnabled(true);
                
                // 2. STEP ONE: Spawn and resume the background network thread immediately!
                fThreadId = spawn_thread(SocatWorker, "socat_worker", B_NORMAL_PRIORITY, this);
                if (fThreadId >= B_OK) {
                    resume_thread(fThreadId);
                }

                // 3. STEP TWO: Now snooze the main UI thread for 400 milliseconds.
                if (srcMarked && (BString(srcMarked->Label()).FindFirst("HTTP") != B_ERROR)) {
                    snooze(400000);
                    
                    // 4. STEP THREE: Open WebPositive.
                    BString systemLaunchCmd;
                    systemLaunchCmd.SetToFormat("open %s &", urlString.String());
                    system(systemLaunchCmd.String());
                }
                break;
            }

            case MSG_ABOUT_HOCAT: {
                BString aboutText;
                aboutText << "HoCat \"Socat\" Controller\n"
                          << "Version 1.0.0  2026 | MIT License\n\n"
                          << "A native Haiku frontend wrapper for network socket testing.\n\n"
                          << "Static Socat Binary:\n"
                          << "• https://github.com/ablyssx74/static-binaries\n\n"
                          << "Blink Viritual Machine:\n"
                          << "• https://github.com/jart/blink\n\n";


                // Instantiate and launch the modal alert box
                BAlert* aboutDialog = new BAlert("About HoCat", aboutText.String(), 
                                                 "OK", nullptr, nullptr, 
                                                 B_WIDTH_AS_USUAL, B_INFO_ALERT);
                
                // setting the shortcut key to enter/escape automatically
                aboutDialog->SetShortcut(0, B_ENTER);
                
                // This displays the dialog box modally and returns when closed
                aboutDialog->Go();
                break;
            }




            case MSG_STOP_SOCAT: {
                KillSocatProcess();
                break;
            }
            case MSG_CLEAR_LOG: {
                fTerminalOutput->SetText("");
                break;
            }
            case MSG_APPEND_TEXT: {
                BString textToAppend;
                if (message->FindString("text", &textToAppend) == B_OK) {
                    int32 textLength = fTerminalOutput->TextLength();
                    fTerminalOutput->Select(textLength, textLength);
                    fTerminalOutput->Insert(textToAppend.String());
                    fTerminalOutput->ScrollToSelection();
                }
                break;
            }
			case MSG_HELP_SOCAT: {
    			fTerminalOutput->SetText("");
    fStartButton->SetEnabled(false);
    fClearButton->SetEnabled(false);
    fHelpButton->SetEnabled(false);
    fStopButton->SetEnabled(true);

    fThreadId = spawn_thread(HelpWorker, "help_worker", B_NORMAL_PRIORITY, this);
    if (fThreadId >= B_OK) {
        resume_thread(fThreadId);
    }
    break;
}

            case MSG_THREAD_DONE: {
                fStartButton->SetEnabled(true);
                fSourceField->SetEnabled(true);
                fDestField->SetEnabled(true);
                fCommandInput->SetEnabled(true);
                fClearButton->SetEnabled(true);
                fHelpButton->SetEnabled(true);
                fStopButton->SetEnabled(false);
                fThreadId = -1;
                break;
            }
            default:
                BWindow::MessageReceived(message);
                break;
        }
    }

private:
    void CheckAndGenerateCert() {
        const char* certPath = "/boot/home/config/settings/hocat/hocat_server.pem";
        
        // Use standard POSIX access() to check if the certificate already exists
        if (access(certPath, F_OK) == 0) {
            return; // Certificate is already present, no need to regenerate!
        }
        
        // Print a notice to the terminal console layout
        fTerminalOutput->SetText("--> Generating Self-Signed SSL Certificate in background...\n");

        // 1. Build the configuration directory folder paths securely
        system("mkdir -p /boot/home/config/settings/hocat");

        // 2. Run OpenSSL to generate the raw 2048-bit RSA keys and certificate layers
        system("openssl req -newkey rsa:2048 -nodes -keyout /tmp/hocat.key -x509 -days 365 -out /tmp/hocat.crt "
               "-subj \"/C=US/ST=Haiku/L=Virtual/O=HoCat/CN=127.0.0.1\" 2>/dev/null");

        // 3. Combine the key and certificate into the single .pem file socat demands
        system("cat /tmp/hocat.key /tmp/hocat.crt > /boot/home/config/settings/hocat/hocat_server.pem");

        // 4. Wipe out the standalone temporary files out of /tmp/
        system("rm -f /tmp/hocat.key /tmp/hocat.crt");
        
        fTerminalOutput->Insert("--> SSL Certificate Generated Successfully!\n\n");
    }

    void UpdateCommandStringFromPresets() {
        BMenuItem* srcMarked = fSourceField->Menu()->FindMarked();
        BMenuItem* dstMarked = fDestField->Menu()->FindMarked();
        
        BString srcParam = "-v -v  -d -d tcp-listen:8080,reuseaddr";
        BString dstParam = "stdout";

        bool isTestMode = false;
        bool isHttpServer = false; // Add a unique flag for the HTTP server payload

        if (srcMarked) {
            BString label = srcMarked->Label();
            if (label.FindFirst("UDP Listener") != B_ERROR) srcParam = "-v -v  -d -d udp-listen:8080";
            else if (label.FindFirst("Standard Input") != B_ERROR) srcParam = "-v -v  -d -d stdin";
            else if (label.FindFirst("input.txt") != B_ERROR) srcParam = "-v -v  -d -d FILE:/boot/home/input.txt";
            
            else if (label.FindFirst("System Info") != B_ERROR) {
                srcParam = "SYSTEM:\"sysinfo\"";
                isTestMode = true;
            }
            else if (label.FindFirst("Network Interfaces") != B_ERROR) {
                srcParam = "SYSTEM:\"ifconfig\"";
                isTestMode = true;
            }
            else if (label.FindFirst("Routing Table") != B_ERROR) {
                srcParam = "SYSTEM:\"route\"";
                isTestMode = true;
            }
            else if (label.FindFirst("HTTP Simple Server") != B_ERROR) {
                // Wrapping the date macro in explicit escaped internal double-quotes (\\\"$(date)\\\")
                // ensures that spaces in the timestamp do not break socat's address parameter parser!
                srcParam = "-v -v  -d -d tcp-listen:8080,crlf,reuseaddr,fork SYSTEM:\"echo HTTP/1.0 200; echo Content-Type\\: text/html; echo; sh -c 'cat << \\\"EOF\\\"\\n<html><body><h1>Welcome to HoCat HTTP Server!</h1><p><b>Current Server Time:</b> $(date) </p></body></html>\\nEOF\\n'\"";
                isHttpServer = true;
            }
            // Map secure memory-backed server
            else if (label.FindFirst("Secure \"HTTPS\" Web Server Test") != B_ERROR || 
                     label.FindFirst("HTTPS Secure Server") != B_ERROR) {
               
                srcParam = "-v -v  -d -d openssl-listen:443,cert=/boot/home/config/settings/hocat/hocat_server.pem,verify=0,crlf,reuseaddr,fork SYSTEM:\"echo HTTP/1.0 200; echo Content-Type\\: text/html; echo; sh -c 'cat << \\\"EOF\\\"\\n<html><body><h1>Welcome to HoCat HTTP Server!</h1><p><b>Current Server Time:</b> $(date) </p></body></html>\\nEOF\\n'\"";
                isHttpServer = true;
            }

        }

        if (dstMarked) {
            BString label = dstMarked->Label();
            if (label.FindFirst("socat.log") != B_ERROR) dstParam = "CREATE:/boot/home/socat.log";
            else if (label.FindFirst("Remote TCP") != B_ERROR) dstParam = "tcp:127.0.0.1:8080";
            else if (label.FindFirst("Serial Interface") != B_ERROR) dstParam = "/dev/ports/usb0";
        }

        BString fullCmd;
        if (isHttpServer) {
            // The HTTP string contains both addresses itself, do not append anything else!
            fullCmd = srcParam;
        } else if (isTestMode) {
            fullCmd.SetToFormat("%s -", srcParam.String());
        } else {
            fullCmd.SetToFormat("%s %s", srcParam.String(), dstParam.String());
        }
        
        fCommandInput->SetText(fullCmd.String());
    }


    static int32 SocatWorker(void* data) {
        SocatWindow* window = static_cast<SocatWindow*>(data);
        
        BString rawArguments;
        window->Lock();
        rawArguments = window->fCommandInput->Text();
        window->Unlock();

        int pipeFDs[2]; 
        if (pipe(pipeFDs) < 0) {
            window->PostMessage(MSG_THREAD_DONE);
            return B_ERROR;
        }

        pid_t pid = fork();
        if (pid < 0) {
            close(pipeFDs[0]);
            close(pipeFDs[1]);
            window->PostMessage(MSG_THREAD_DONE);
            return B_ERROR;
        }

        if (pid == 0) {
            // --- INSIDE CHILD PROCESS ---
            setpgid(0, 0); 

            // Route standard output and error descriptors to the pipeline
            dup2(pipeFDs[1], STDOUT_FILENO);
            dup2(pipeFDs[1], STDERR_FILENO);
            close(pipeFDs[0]);
            close(pipeFDs[1]);

            // Clean Reversion: Execute blink socat directly without grep filtering blocks
            execlp("sh", "sh", "-c", BString().SetToFormat("exec blink -j socat %s", rawArguments.String()).String(), nullptr);
            _exit(1);
        }

        close(pipeFDs[1]); 
        
        window->Lock();
        window->fSocatTeamId = pid;
        window->Unlock();

        FILE* stream = fdopen(pipeFDs[0], "r");
        if (stream) {
            char buffer[256]; 
            while (fgets(buffer, sizeof(buffer), stream) != nullptr) {
                BMessage* msg = new BMessage(MSG_APPEND_TEXT);
                msg->AddString("text", buffer);
                window->PostMessage(msg);
            }
            fclose(stream);
        }
        close(pipeFDs[0]);

        window->PostMessage(MSG_THREAD_DONE);
        return B_OK;
    }

    // New worker dedicated to calling the underlying help documentation safely
    static int32 HelpWorker(void* data) {
        SocatWindow* window = static_cast<SocatWindow*>(data);

        int pipeFDs[2]; 
        if (pipe(pipeFDs) < 0) {
            window->PostMessage(MSG_THREAD_DONE);
            return B_ERROR;
        }

        pid_t pid = fork();
        if (pid < 0) {
            close(pipeFDs[0]);
            close(pipeFDs[1]);
            window->PostMessage(MSG_THREAD_DONE);
            return B_ERROR;
        }

        if (pid == 0) {
            setpgid(0, 0); 
            dup2(pipeFDs[1], STDOUT_FILENO);
            dup2(pipeFDs[1], STDERR_FILENO);
            close(pipeFDs[0]);
            close(pipeFDs[1]);

            execlp("sh", "sh", "-c", "exec blink socat -h", nullptr);
            _exit(1);
        }

        close(pipeFDs[1]); 
        
        window->Lock();
        window->fSocatTeamId = pid;
        window->Unlock();

        FILE* stream = fdopen(pipeFDs[0], "r");
        if (stream) {
            char buffer[256]; 
            while (fgets(buffer, sizeof(buffer), stream) != nullptr) {
                BMessage* msg = new BMessage(MSG_APPEND_TEXT);
                msg->AddString("text", buffer);
                window->PostMessage(msg);
            }
            fclose(stream);
        }
        close(pipeFDs[0]);

        window->PostMessage(MSG_THREAD_DONE);
        return B_OK;
    }

    void KillSocatProcess() {
        if (fSocatTeamId >= 0) {
            kill(-fSocatTeamId, SIGINT); 
            fSocatTeamId = -1;
        }
    }

    BMenuField*   fSourceField;
    BMenuField*   fDestField;
    BTextControl* fCommandInput;
    BButton*      fStartButton;
    BButton*      fStopButton;
    BButton*      fClearButton; 
    BButton*      fHelpButton; 
    BButton*      fAboutButton; 
    BTextView*    fTerminalOutput;
    thread_id     fThreadId;
    pid_t         fSocatTeamId;
}; // End of SocatWindow class

class SocatApp : public BApplication {
public:
    SocatApp() : BApplication("application/x-vnd.HaikuSocatGui") {}

    void ReadyToRun() override {
        SocatWindow* window = new SocatWindow();
        window->Show();
    }
};

int main() {
    SocatApp app;
    app.Run();
    return 0;
}
