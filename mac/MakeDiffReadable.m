#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import <unistd.h>
#import "../Reader.h"
#import "../appIcon.png.h"

@interface MDRApplicationDelegate : NSObject
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*) sender;
- (void) closeKeyWindow:(id) sender;
@end

@interface MDRServer : NSObject {
    NSMutableArray * windowList;
}
- (void) setWindowList:(NSMutableArray *) list;
- (BOOL) showWindowWithContent:(NSString *) displayContent;
@end

int main (int argc, const char * argv[])
{

    char * html;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--html") == 0)
        {
            html = getHTML();
            printf("%s", html);
            free(html);
        }
        else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)
        {
            printf("%s", getVersion());
        }
        else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
        {
            printf("%s", getHelp());
        }
        else
        {
            printf("Unknown arguments.\n%s", getHelp());
        }
        return 0;
    }
    else
    {
        // Calling getHTML will grab text from stdin. We want to do this before
        // we fork in case there isn't any so the user can enter it manually.
        html = getHTML();
    }

    pid_t pid = fork();
    if (pid)
    {
        // Parent process just quits after fork.
        free(html);
        return 0;
    }
    else
    {
        // Child process.
        setsid();

        id pool = [NSAutoreleasePool new];
        NSString * displayContent = [NSString stringWithCString:html encoding:NSUTF8StringEncoding];
        free(html);

        id connectionName = @"com.halffullheart.mdr.diffserverconnection";
        NSConnection * connection = [[NSConnection connectionWithRegisteredName:connectionName host:nil] autorelease];
        NSProxy * proxy = [[connection rootProxy] autorelease];

        if (proxy)
        {
            // Connected to server - act as client.
            [(MDRServer *)proxy showWindowWithContent:displayContent];
            [connection invalidate];
            [pool drain];
        }
        else
        {
            [NSApplication sharedApplication];
            [NSApp setDelegate:[MDRApplicationDelegate alloc]];

            //NSLog(@"Server mode.");
            // No connection - act as server.

            // Transform into "real" app with dock icon and menubar.
            ProcessSerialNumber psn = { 0, kCurrentProcess };
            OSStatus returnCode = TransformProcessType(&psn, kProcessTransformToForegroundApplication);
            if (returnCode != 0) {
                printf("Could not bring the application to front. Error %d", (int)returnCode);
            }

            // Set app icon.
            NSImage * appIcon = [[[NSImage alloc] initWithData:[NSData dataWithBytes:appIcon_png length:appIcon_png_len]] autorelease];
            [NSApp setApplicationIconImage: appIcon];

            // Set up menus.
            id menubar = [[NSMenu new] autorelease];
            [NSApp setMainMenu:menubar];

            // App menu.
            id appMenuItem = [[NSMenuItem new] autorelease];
            [menubar addItem:appMenuItem];
            id appMenu = [[NSMenu new] autorelease];
            id appName = [[NSProcessInfo processInfo] processName];
            id quitTitle = [@"Quit " stringByAppendingString:appName];
            id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
                action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
            [appMenu addItem:quitMenuItem];
            [appMenuItem setSubmenu:appMenu];

            // File menu.
            id fileMenuItem = [[NSMenuItem new] autorelease];
            [menubar addItem:fileMenuItem];
            id fileMenu = [[[NSMenu alloc] initWithTitle:@"File"] autorelease];
            id closeWindowItem = [[[NSMenuItem alloc] initWithTitle:@"Close Window"
                action:@selector(closeKeyWindow:) keyEquivalent:@"w"] autorelease];
            [fileMenu addItem:closeWindowItem];
            [fileMenuItem setSubmenu:fileMenu];

            // Window menu.
            id windowMenuItem = [[NSMenuItem new] autorelease];
            [menubar addItem:windowMenuItem];
            id windowMenu = [[[NSMenu alloc] initWithTitle:@"Window"] autorelease];
            [windowMenuItem setSubmenu:windowMenu];
            [NSApp setWindowsMenu:windowMenu];

            // Set up server.
            id server = [[[MDRServer alloc] init] autorelease];

            // Keep list of windows so they are still in memory in the main app loop.
            id windowList = [[[NSMutableArray alloc] init] autorelease];
            [server setWindowList:windowList];

            NSConnection * serverConnection = [[NSConnection new] autorelease];
            [serverConnection setRootObject:server];
            [serverConnection registerName:connectionName];

            // Have the server launch a window with the output for this time.
            [server showWindowWithContent:displayContent];

            [NSApp activateIgnoringOtherApps:YES];
            [NSApp run];

        }

        return 0;
    }

}

@implementation MDRApplicationDelegate

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*) sender
{
    return YES;
}

- (void) closeKeyWindow:(id) sender
{
    [[NSApp keyWindow] close];
}

@end

@implementation MDRServer

- (void) setWindowList:(NSMutableArray *) list
{
    windowList = list;
}

- (BOOL) showWindowWithContent:(NSString *) displayContent
{
    //NSLog(@"Client contacted me");
    NSRect startingWindowSize = NSMakeRect(0, 0, 900, 600);

    id window = [[[NSWindow alloc] initWithContentRect:startingWindowSize
        styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask)
        backing:NSBackingStoreBuffered defer:NO] autorelease];

    [window cascadeTopLeftFromPoint:NSMakePoint(200,200)];
    id windowTitle = [@"mdr " stringByAppendingString:[NSString stringWithFormat:@"%d", [windowList count] + 1]];
    [window setTitle:windowTitle];
    [window makeKeyAndOrderFront:nil];

    id webView = [[[WebView alloc] initWithFrame:startingWindowSize] autorelease];
    [window setContentView:webView];
    [[webView mainFrame] loadHTMLString:displayContent baseURL: nil];

    [windowList addObject:window];
    return YES;
}

@end
