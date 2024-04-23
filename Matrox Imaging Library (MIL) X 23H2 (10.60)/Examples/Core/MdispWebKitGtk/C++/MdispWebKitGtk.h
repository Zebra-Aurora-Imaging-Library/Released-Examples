//
// Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MDISPWEBKITGTK_H
#define MDISPWEBKITGTK_H
#include <gtk/gtk.h>
#if WEBKIT2
#include<webkit2/webkit2.h>
#else
#include<webkit/webkit.h>
#endif
typedef struct  MilWindow
   {
      GtkWidget* Window;
      GtkWidget* Web;
      GtkWidget* Entry;
   } MilWindow;
#endif // MDISPWEBKITGTK_H
