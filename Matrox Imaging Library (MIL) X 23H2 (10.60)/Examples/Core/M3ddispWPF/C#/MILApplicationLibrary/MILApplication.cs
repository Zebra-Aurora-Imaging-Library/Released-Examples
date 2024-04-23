using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;

using Matrox.MatroxImagingLibrary;

//********************************************************************************
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//********************************************************************************

namespace MILApplicationLibrary
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    /// An object that encapsulates the functionality of a simple MIL application.
    /// </summary>
    /// <remarks>
    /// <list type="bullet">
    /// <item>This object implements the <see cref="INotifyPropertyChanged"/> interface to allow data binding.</item>
    /// </list>
    /// </remarks>
    public class MILApplication : INotifyPropertyChanged
    {
        #region Constants

        // Default image dimensions.
        private const int DEFAULT_IMAGE_SIZE_X = 640;
        private const int DEFAULT_IMAGE_SIZE_Y = 480;
        private const int DEFAULT_IMAGE_SIZE_BAND = 1;

        #endregion

        #region Constructor

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Creates a new unallocated MILApplication object.
        /// </summary>
        public MILApplication()
        {
            _appId = MIL.M_NULL;
            _sysId = MIL.M_NULL;
            _digId = MIL.M_NULL;
            _dispId = MIL.M_NULL;
            _containerId = MIL.M_NULL;
        }

        #endregion

        #region Public methods

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Allocates a MIL application, system, display, image buffer and digitizer (if available).
        /// </summary>
        public void Allocate()
        {
            // Allocate a MIL application.
            MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref _appId);

            // Allocate a MIL system.
            MIL.MsysAlloc(MIL.M_DEFAULT, "M_SYSTEM_HOST", MIL.M_DEFAULT, MIL.M_DEFAULT, ref _sysId);

            // Allocate a MIL display.
            MIL.M3ddispAlloc(_sysId, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WPF, ref _dispId);

            // Set default values for the image buffer in case no digitizer can be allocated.
            MIL_INT bufferSizeX = DEFAULT_IMAGE_SIZE_X;
            MIL_INT bufferSizeY = DEFAULT_IMAGE_SIZE_Y;
            MIL_INT bufferSizeBand = DEFAULT_IMAGE_SIZE_BAND;
            long imageAttributes = MIL.M_IMAGE | MIL.M_DISP | MIL.M_PROC;

            // Inquire the number of digitizers for the system.
            MIL_INT numberOfDigitizers = MIL.MsysInquire(_sysId, MIL.M_DIGITIZER_NUM, MIL.M_NULL);
            if (numberOfDigitizers > 0)
            {
                // Allocate a digitizer.
                MIL.MdigAlloc(_sysId, MIL.M_DEFAULT, "M_3D_SIMULATOR", MIL.M_DEFAULT, ref _digId);

                // Add the M_GRAB attibute to the image buffer.
                imageAttributes |= MIL.M_GRAB;
            }

            // Notify the UI that grabbing options have changed.
            RaisePropertyChangedEvent("CanStartGrab");
            RaisePropertyChangedEvent("CanStopGrab");

            // Allocate the container.
            MIL.MbufAllocContainer(_sysId, MIL.M_PROC | MIL.M_GRAB | MIL.M_DISP, MIL.M_DEFAULT, ref _containerId);

            // Notify the UI that the buffer size changed.
            RaisePropertyChangedEvent("BufferSizeX");
            RaisePropertyChangedEvent("BufferSizeY");

            // Do one grab to fill the container.
            MIL.MdigGrab(_digId, _containerId);

            // Select the 3D container to display.
            MIL.M3ddispSelect(_dispId, _containerId, MIL.M_DEFAULT, MIL.M_DEFAULT);

            }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Starts the grab on the digitizer.
        /// </summary>
        /// <remarks>
        /// This method is called from the StartGrab_Click method of the main window when the 
        /// user clicks the Stop Grab button in the UI.
        /// </remarks>
        public void StartGrab()
        {
            MIL.MdigGrabContinuous(_digId, _containerId);

            _isGrabbing = true;

            // Notify the UI that grabbing options have changed.
            RaisePropertyChangedEvent("CanStartGrab");
            RaisePropertyChangedEvent("CanStopGrab");
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Stops the grab on the digitizer.
        /// </summary>
        /// <remarks>
        /// This method is called from the StopGrab_Click method of the main window when the 
        /// user clicks the Stop Grab button in the UI.
        /// </remarks>
        public void StopGrab()
        {
            MIL.MdigHalt(_digId);

            _isGrabbing = false;

            // Notify the UI that grabbing options have changed.
            RaisePropertyChangedEvent("CanStartGrab");
            RaisePropertyChangedEvent("CanStopGrab");
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Frees the MIL Application and its associated resources.
        /// </summary>
        public void Free()
        {
            // Stop the grab if necessary.
            if (CanStopGrab)
            {
                StopGrab();
            }

            // Free the display.
            if (_dispId != MIL.M_NULL)
            {
                MIL.M3ddispFree(_dispId);
                _dispId = MIL.M_NULL;
            }

            // Free the container.
            if (_containerId != MIL.M_NULL)
            {
                MIL.MbufFree(_containerId);
                _containerId = MIL.M_NULL;
            }

            // Free the digitizer.
            if (_digId != MIL.M_NULL)
            {
                MIL.MdigFree(_digId);
                _digId = MIL.M_NULL;
            }

            // Free the system.
            if (_sysId != MIL.M_NULL)
            {
                MIL.MsysFree(_sysId);
                _sysId = MIL.M_NULL;
            }

            // Free the application.
            if (_appId != MIL.M_NULL)
            {
                MIL.MappFree(_appId);
                _appId = MIL.M_NULL;
            }

            // The object has been cleaned up.
            // This call removes the object from the finalization queue and 
            // prevent finalization code object from executing a second time.
            GC.SuppressFinalize(this);
        }

        #endregion

        #region Properties used in data bindings

      public MIL_ID MilDisplayId
         {
         get
            {
            return _dispId;
            }
         }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Returns whether or not the application can start grabbing live images.
        /// </summary>
        /// <remarks>This property is bound to the IsEnabled property of the Start Grab button.</remarks>
        public bool CanStartGrab
        {
            get { return ((_digId != MIL.M_NULL) && (!_isGrabbing)); }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Returns whether or not the application can stop grabbing live images.
        /// </summary>
        /// <remarks>This property is bound to the IsEnabled property of the Stop Grab button.</remarks>
        public bool CanStopGrab
        {
            get { return ((_digId != MIL.M_NULL) && (_isGrabbing)); }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Returns the width of the displayed image buffer.
        /// </summary>
        /// <remarks>This property is bound to the Width property of the WindowsFormsHost control.</remarks>
        public int BufferSizeX
        {
            get { return DEFAULT_IMAGE_SIZE_X; }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Returns the height of the displayed image buffer.
        /// </summary>
        /// <remarks>This property is bound to the Height property of the WindowsFormsHost control.</remarks>
        public int BufferSizeY
        {
            get { return DEFAULT_IMAGE_SIZE_Y; }
        }

        #endregion

        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion

        #region Helper functions

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Raises the <see cref="PropertyChanged"/> event for the specified property.
        /// </summary>
        /// <param name="propertyName">A <see cref="string"/> object representing the name of the property that changed.</param>
        /// <remarks>Call this method to notify the UI that a property changed.</remarks>
        private void RaisePropertyChangedEvent(string propertyName)
        {
            // In debug builds, make sure the property exists in this object.
            VerifyPropertyName(propertyName);

            // Raise the PropertyChanged event.
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////
        /// <summary>
        /// Warns the developer if this object does not have a public property with the specified name.
        /// </summary>
        /// <param name="propertyName">The name of the property to verify.</param>
        /// <remarks>This method does not exist in a Release build.</remarks>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        public void VerifyPropertyName(string propertyName)
        {
            // Verify that the property name matches a real, 
            // public, instance property on this object.
            if (TypeDescriptor.GetProperties(this)[propertyName] == null)
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        #endregion

        #region Private members

        private MIL_ID _appId;
        private MIL_ID _sysId;
        private MIL_ID _digId;
        private MIL_ID _dispId;
        private MIL_ID _containerId;
        private bool _isGrabbing;

        #endregion
    }
}
