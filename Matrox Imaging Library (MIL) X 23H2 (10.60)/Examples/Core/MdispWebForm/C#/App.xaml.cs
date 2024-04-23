using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Reflection;
using System.Windows;
using Microsoft.Win32;

//********************************************************************************
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//********************************************************************************

namespace MdispWebForm
   {
   /// <summary>
   /// Interaction logic for App.xaml
   /// </summary>
   public partial class App : Application
      {
      private void Application_Startup(object sender, StartupEventArgs e)
         {
         MainWindow wnd;

         // First add registry key, if it doesn't already exist.  Necessary due to limitations with IE about websockets
         // (https://msdn.microsoft.com/en-us/library/ee330736(v=vs.85).aspx#websocket_maxconn)
         string subKeyName = @"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Internet Explorer\MAIN\FeatureControl\FEATURE_WEBSOCKET_MAXCONNECTIONSPERSERVER";
         string valueName = Assembly.GetExecutingAssembly().GetName().Name + ".exe";
         SetRegistryValue(subKeyName, valueName, 0x00000080, RegistryValueKind.DWord);

         // Then instantiate MainWindow (passing url to constructor, if given by user)
         if (e.Args.Length == 1)
            {
            wnd = new MainWindow(e.Args[0]);
            }
         else
            {
            wnd = new MainWindow();
            }

         MainWindow = wnd;
         wnd.Show();
         }

      public void SetRegistryValue(string registrySubKeyName, string valueName, object valueToSet, RegistryValueKind registryKind)
         {
         try
            {
            // Get current value in registry (if it already exists)
            object currentValueInRegistry = Registry.GetValue(registrySubKeyName, valueName, null);

            // If it doesn't exist, or a different value, we (re)create it
            if (!valueToSet.Equals(currentValueInRegistry))
               {
               Registry.SetValue(registrySubKeyName, valueName, valueToSet, registryKind);
               }
            }

         catch (Exception e)
            {
            Console.WriteLine("Exception occurred: " + e.Message);
            }
         }
      }
   }
