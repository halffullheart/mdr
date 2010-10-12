#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "Reader.h"

int main ()
{
    [NSAutoreleasePool new];
    [NSApplication sharedApplication];
    id appName = [[NSProcessInfo processInfo] processName];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];

    char* body = getHTML();
    NSString* html = [NSString stringWithCString:body encoding:NSASCIIStringEncoding];
    free(body);

    NSRect rect = NSMakeRect(0, 0, 640, 480);

    id window = [[[NSWindow alloc] initWithContentRect:rect
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO]
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
