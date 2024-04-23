using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Navigation;
using System.Windows.Shapes;

//********************************************************************************
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//********************************************************************************

namespace MdispWebForm
   {
   /// <summary>
   /// Interaction logic for MainWindow.xaml
   /// </summary>
   public partial class MainWindow : Window
      {
      public MainWindow(string url)
         {
         InitializeComponent();
         this.URLBox.Text = url;
         this.Title = "MdispWebForm";
         }

      public MainWindow()
         {
         InitializeComponent();
         this.URLBox.Text = null;
         this.Title = "MdispWebForm";
         }

      private void Window_Loaded(object sender, RoutedEventArgs e)
         {
         NavigateToWebPage(this.URLBox.Text);
         }

      private void Button_Click(object sender, RoutedEventArgs e)
         {
         NavigateToWebPage(this.URLBox.Text);
         }

      private void NavigateToWebPage(string url)
         {
         if (Uri.IsWellFormedUriString(URLBox.Text, UriKind.Absolute))
            {
            Uri uri = new Uri(URLBox.Text);
            MainBrowser.Navigate(uri);
            StatusBarTextBlock.Foreground = Brushes.Black;
            StatusBarTextBlock.Text = "";
            }
         else if (string.IsNullOrEmpty(URLBox.Text))
            {
            StatusBarTextBlock.Foreground = Brushes.Black;
            StatusBarTextBlock.Text = "Please enter a valid URL";
            }
         else
            {
            StatusBarTextBlock.Foreground = Brushes.Red;
            StatusBarTextBlock.Text = "Error: Please enter valid URL";
            }
         }

      private void MainBrowser_Navigated(object sender, NavigationEventArgs e)
         {
         if (URLBox.Text != e.Uri.AbsoluteUri)
            {
            URLBox.Text = e.Uri.AbsoluteUri;
            }
         }

      private void URLBox_KeyDown(object sender, KeyEventArgs e)
         {
         if (e.Key == Key.Enter)
            {
            NavigateToWebPage(this.URLBox.Text);
            }
         }
      }
   }
