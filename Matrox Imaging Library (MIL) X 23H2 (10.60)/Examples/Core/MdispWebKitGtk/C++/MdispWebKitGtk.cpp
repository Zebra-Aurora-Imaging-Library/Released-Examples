//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#include "MdispWebKitGtk.h"

#define BROWSER_SIZE_X 800
#define BROWSER_SIZE_Y 800

/***************************************************************************
 * Name:         gtk_go_callback
 *
 * synopsis:     Called when user click Go button.
 *
 *
 */

void gtk_go_callback(GtkWidget       *widget,
                         gpointer         data)
   {
   MilWindow *MainWindow=(MilWindow *)data;
   const gchar *uri = gtk_entry_get_text(GTK_ENTRY(MainWindow->Entry));
   gtk_entry_progress_pulse(GTK_ENTRY(MainWindow->Entry));
   webkit_web_view_load_uri(WEBKIT_WEB_VIEW(MainWindow->Web),uri);
   }

/***************************************************************************
 * Name:         main application (Gtk3)
 *
 * synopsis:     Create widgets attach callbacks and loop.
 *
 *
 */

int main(int argc, char *argv[])
   {

   /* some gtkwidgets */
   GtkWidget *button = NULL;
   GtkWidget *vbox   = NULL;
   GtkWidget *hbox   = NULL;
   GtkWidget *label  = NULL;
   MilWindow MainWindow;
   
   const gchar *url = "http://localhost:9001/mdispweb.html";
   if(argc > 1)
      url  = argv[1];
   
   gtk_init(&argc, &argv);
   
   /* main window */
   MainWindow.Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   
   gtk_window_set_title(GTK_WINDOW(MainWindow.Window), "MdispWebKitGtk");
   g_signal_connect (G_OBJECT (MainWindow.Window), "destroy", gtk_main_quit,NULL);
   gtk_window_set_default_size(GTK_WINDOW(MainWindow.Window),BROWSER_SIZE_X, BROWSER_SIZE_Y);
   vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
   gtk_container_add (GTK_CONTAINER (MainWindow.Window), vbox);

   hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
   gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
   label = gtk_label_new("Url :");
   gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
   MainWindow.Entry = gtk_entry_new();
   gtk_entry_set_text (GTK_ENTRY(MainWindow.Entry),url);
   gtk_box_pack_start (GTK_BOX (hbox), MainWindow.Entry, TRUE, TRUE, 0);
   button=gtk_button_new_with_label ("Go To");
   gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
   g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gtk_go_callback), (gpointer) &MainWindow);
   
   
   MainWindow.Web = webkit_web_view_new();
   webkit_web_view_load_uri (WEBKIT_WEB_VIEW(MainWindow.Web),url);
   gtk_box_pack_start(GTK_BOX(vbox),MainWindow.Web,TRUE,TRUE,0);

   gtk_widget_show_all (MainWindow.Window);

   /* enter the GTK main loop */
   gtk_main();
   return 0;
   }

