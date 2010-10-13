#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "Reader.h"

@interface MDRApplicationDelegate : NSObject
@end

int main ()
{

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

        char* body = getHTML();
        NSString* html = [NSString stringWithCString:body encoding:NSASCIIStringEncoding];
        free(body);

        NSRect rect = NSMakeRect(0, 0, 640, 480);

        id window = [[[NSWindow alloc] initWithContentRect:rect
            styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask)
            backing:NSBackingStoreBuffered defer:NO]
                autorelease];

        [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
        [window setTitle:appName];
        [window makeKeyAndOrderFront:nil];
        [window orderFrontRegardless];

        id webView = [[[WebView alloc] initWithFrame:rect] autorelease];
        [window setContentView:webView];
        [[webView mainFrame] loadHTMLString:html baseURL: nil];

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
