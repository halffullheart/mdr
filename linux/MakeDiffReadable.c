#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include "../Reader.h"
#include "../appIcon.png.h"

gboolean disabledContextMenu() {
  return TRUE;
}

GdkPixbuf * buildApplicationIcon() {
    GdkPixbufLoader *appIconLoader;
    GError *err = NULL;

    appIconLoader = gdk_pixbuf_loader_new_with_type("png", &err);
    if (err != NULL) {
        fprintf(stderr, "Error creating PNG loader: %s\n", err->message);
        g_error_free(err);
    }

    gdk_pixbuf_loader_write(appIconLoader, appIcon_png, appIcon_png_len, &err);
    if (err != NULL) {
        fprintf(stderr, "Error reading icon data: %s\n", err->message);
        g_error_free(err);
    }

    gdk_pixbuf_loader_close(appIconLoader, &err);
    if (err != NULL) {
        fprintf(stderr, "Error closing PNG loader: %s\n", err->message);
        g_error_free(err);
    }

    return gdk_pixbuf_loader_get_pixbuf(appIconLoader);
}

int main (int argc, char * argv[])
{

    char * html;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--html") == 0)
        {
            html = getHtmlFromStdIn();
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
        GtkWidget *scrolledWindow; // TODO: remove when switching to webkit2
        GdkPixbuf *appIcon;
        WebKitWebView *webView;

        gtk_init(&argc, &argv);

        mainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
        webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
        appIcon = buildApplicationIcon();

        gtk_window_set_default_size(GTK_WINDOW(mainWin), 900, 600);
        gtk_window_set_title(GTK_WINDOW(mainWin), "mdr");
        gtk_window_set_icon(GTK_WINDOW(mainWin), appIcon);
        g_signal_connect (mainWin, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        g_signal_connect (webView, "context-menu", G_CALLBACK(disabledContextMenu), NULL);
        gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(webView));
        gtk_container_add(GTK_CONTAINER(mainWin), scrolledWindow);

        html = getHtmlFromStdIn();
        webkit_web_view_load_html_string(webView, html, NULL);

        gtk_widget_grab_focus(GTK_WIDGET(webView));
        gtk_widget_show_all(mainWin);

        gtk_main();

        free(html);
    }

    return 0;
}
