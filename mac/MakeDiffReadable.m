#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "../Reader.h"

@interface MDRApplicationDelegate : NSObject
@end

int main (int argc, const char * argv[])
{

    char * html = getHTML();

    if (argc > 1 && strcmp(argv[1], "--html") == 0)
    {
        printf("HTML Output:\n%s\n", html);
        free(html);
        return 0;
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        setsid();

        [NSAutoreleasePool new];
        [NSApplication sharedApplication];
        id appName = [[NSProcessInfo processInfo] processName];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        [NSApp setDelegate:[MDRApplicationDelegate alloc]];

        NSString* webContent = [NSString stringWithCString:html encoding:NSASCIIStringEncoding];
        free(html);

        NSRect rect = NSMakeRect(0, 0, 1000, 500);

        id window = [[[NSWindow alloc] initWithContentRect:rect
            styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask)
            backing:NSBackingStoreBuffered defer:NO]
                autorelease];

        [window cascadeTopLeftFromPoint:NSMakePoint(200,200)];
        [window setTitle:appName];
        [window makeKeyAndOrderFront:nil];
        [window orderFrontRegardless];

        id webView = [[[WebView alloc] initWithFrame:rect] autorelease];
        [window setContentView:webView];
        [[webView mainFrame] loadHTMLString:webContent baseURL: nil];

        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];

        return 0;
    }
    else
    {
        // Parent process
        return 0;
    }

}

@implementation MDRApplicationDelegate
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*) sender
{
    return YES;
}
@end
