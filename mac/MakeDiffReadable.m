#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import <unistd.h>
#import "../Reader.h"

@interface MDRApplicationDelegate : NSObject
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
        return 0;
    }
    else
    {
        // Child process.
        setsid();

        [NSAutoreleasePool new];
        [NSApplication sharedApplication];
        [NSApp setDelegate:[MDRApplicationDelegate alloc]];

        NSString* webContent = [NSString stringWithCString:html encoding:NSUTF8StringEncoding];
        free(html);

        NSRect rect = NSMakeRect(0, 0, 900, 600);

        id window = [[[NSWindow alloc] initWithContentRect:rect
            styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask)
            backing:NSBackingStoreBuffered defer:NO]
                autorelease];

        [window cascadeTopLeftFromPoint:NSMakePoint(200,200)];
        [window setTitle:@"mdr"];
        [window makeKeyAndOrderFront:nil];
        [window orderFrontRegardless];

        id webView = [[[WebView alloc] initWithFrame:rect] autorelease];
        [webView setEditable:YES];
        [window setContentView:webView];
        [[webView mainFrame] loadHTMLString:webContent baseURL: nil];

        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];

        return 0;
    }

}

@implementation MDRApplicationDelegate
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*) sender
{
    return YES;
}
@end
