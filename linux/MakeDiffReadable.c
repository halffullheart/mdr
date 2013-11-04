#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include "../Reader.h"

int main (int argc, char * argv[])
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
        GtkWidget *mainWin;
        WebKitWebView *webView;

        gtk_init(&argc, &argv);

        mainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

        gtk_window_set_default_size(GTK_WINDOW(mainWin), 900, 600);
        gtk_window_set_title(GTK_WINDOW(mainWin), "mdr");
        g_signal_connect (mainWin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        gtk_container_add(GTK_CONTAINER(mainWin), GTK_WIDGET(webView));

        html = getHTML();
        webkit_web_view_load_html(webView, html, NULL);

        gtk_widget_grab_focus(GTK_WIDGET(webView));
        gtk_widget_show_all(mainWin);

        gtk_main();

        free(html);
    }

    return 0;
}
