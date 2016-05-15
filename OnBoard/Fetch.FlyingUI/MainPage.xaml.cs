﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Graphics.Display;
using Windows.Media.Capture;
using Windows.Media.MediaProperties;
using Windows.Networking.Sockets;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.System.Threading;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Documents;
using Windows.UI.Xaml.Media;
using Adafruit.Pwm;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace Fetch.FlyingUI
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private PwmController _hat;

        private HttpServer _httpServer;

        private MediaCapture _mediaCapture;

        public Stream CameraStream;

        public StorageFile photoFile;
        public readonly string PHOTO_FILE_NAME = "photo.jpg";


        public void SpawnServer()
        {
            _httpServer = new HttpServer(8000, this);
            _httpServer.StartServer();
        }

        public MainPage()
        {
            Debug.WriteLine("Package Family Name:\n  " + Package.Current.Id.FamilyName + "\n");

            // Init UI
            InitializeComponent();

            // Unit the server
            SpawnServer();

            // Init the camera
            CameraInit();

            // Init the hat
            _hat = new PwmController();
            _hat.SetDesiredFrequency(364); // value determined experimentally.  See notebook for targets in Hz. 


            Task.Factory.StartNew(() =>
            {
                // start the status beacon
                int inc = 1;
                int lev = 2048;
                
                while (true)
                {
                    if (lev <= 1 || lev >= 4095)
                        inc = -inc;

                    if (inc > 0)
                        lev = lev << 1;
                    else
                        lev = lev >> 1;
                
                    SetPWM(15, 4096-lev);
                    Task.Delay(TimeSpan.FromMilliseconds(20)).Wait();
                }
            });



        }

        private UInt16[] _pwmValues = new UInt16[16];


        public string getPwmString()
        {
            string res = "";
            for (int i = 0; i < _pwmValues.Length; i++)
            {
                res += getPwmSingleString(i);
            }
            return res;
        }

        private string getPwmSingleString(int which)
        {
            return _pwmValues[which].ToString("X3")
                   + (which + 1 == _pwmValues.Length ? "" : ",");
        }

        public void SetPWM(int which, int setTo)
        {
            setTo = (setTo > 4094 ? 4094 : setTo);
            setTo = (setTo < 1 ? 1 : setTo);

            _pwmValues[which] = (ushort)setTo;

            if (which > 15 || which < 0)
                throw new ArgumentOutOfRangeException(nameof(which), "Must be 0-15, inclusive.");

            _hat.SetPulseParameters(which, setTo);
        }


        private async void CameraInit()
        {
            try
            {
                if (_mediaCapture != null)
                {
                    // Cleanup MediaCapture object
                    CameraCleanup();
                }

//                Task.Delay(TimeSpan.FromSeconds(10)).Wait();


                Debug.WriteLine("Initing Video Capture... ");
                // Use default-ish initialization
                MediaCaptureInitializationSettings capInitSettings = new MediaCaptureInitializationSettings();
                capInitSettings.StreamingCaptureMode = StreamingCaptureMode.Video;
                _mediaCapture = new MediaCapture();
                _mediaCapture.Failed += new MediaCaptureFailedEventHandler(CameraInit_Failed);
                await _mediaCapture.InitializeAsync(capInitSettings);


                IReadOnlyList<IMediaEncodingProperties> resolutions = _mediaCapture.VideoDeviceController.GetAvailableMediaStreamProperties(MediaStreamType.VideoPreview);

                VideoEncodingProperties bestRes = null;

                // search for minimal available frame rate and resolution. 
                Debug.WriteLine("Available Resolutions: ");
                for (var i = 0; i < resolutions.Count; i++)
                {
                    if (resolutions[i].GetType().Name != "VideoEncodingProperties")
                        continue;
                    var r = (VideoEncodingProperties) resolutions[i];

                    Debug.WriteLine(i.ToString() + " res: " + r.Width + "x" + r.Height + " @ " + r.FrameRate.Numerator + "/" + r.FrameRate.Denominator + " fps (" + r.Type + ": " + r.Subtype + ")");

                    if (r.FrameRate.Denominator != 1)
                        continue;

                    if (bestRes == null)
                    {
                        bestRes = r;
                        continue;
                    }

                    if (r.FrameRate.Numerator <= bestRes.FrameRate.Numerator && 
                        r.Height * r.Width <= bestRes.Height * bestRes.Width &&
                        r.Height >= 400 &&
                        r.Subtype == "YUY2")
                    {
                        bestRes = r;
                    }

                }

                await _mediaCapture.VideoDeviceController.SetMediaStreamPropertiesAsync(MediaStreamType.Photo, bestRes);

                Debug.WriteLine("Using: " + bestRes.Width + "x" + bestRes.Height + " @ " + bestRes.FrameRate.Numerator + "/" + bestRes.FrameRate.Denominator + " fps");


                PreviewElement.Source = _mediaCapture;
                PreviewElement.Stretch = Stretch.None;
                await _mediaCapture.StartPreviewAsync();

                Debug.WriteLine("done.");
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Unable to initialize camera for audio/video mode: " + ex.Message);
            }
        }


        /// <summary>
        /// Callback function for any failures in MediaCapture operations
        /// </summary>
        /// <param name="currentCaptureObject"></param>
        /// <param name="currentFailure"></param>
        private async void CameraInit_Failed(MediaCapture currentCaptureObject, MediaCaptureFailedEventArgs currentFailure)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                try
                {
                    Debug.WriteLine("MediaCaptureFailed: " + currentFailure.Message);

//                    if (isRecording)
//                    {
//                        await mediaCapture.StopRecordAsync();
//                        status.Text += "\n Recording Stopped";
//                    }
                }
                catch (Exception)
                {
                }
                finally
                {
//                    SetInitButtonVisibility(Action.DISABLE);
//                    SetVideoButtonVisibility(Action.DISABLE);
//                    SetAudioButtonVisibility(Action.DISABLE);
//                    status.Text += "\nCheck if camera is diconnected. Try re-launching the app";
                }
            });
        }


        private void CameraCleanup()
        {
            if (_mediaCapture != null)
            {
                // Cleanup MediaCapture object
                _mediaCapture.Dispose();
                _mediaCapture = null;
            }
        }

        public async void CameraTake()
        {
            photoFile = await KnownFolders.PicturesLibrary.CreateFileAsync(
                    PHOTO_FILE_NAME, CreationCollisionOption.GenerateUniqueName);
            ImageEncodingProperties imageProperties = ImageEncodingProperties.CreateJpeg();
            ((Task)_mediaCapture.CapturePhotoToStorageFileAsync(imageProperties, photoFile)).Wait();
           
            Debug.WriteLine("Take Photo to file succeeded: " + photoFile.Path);
        }

//        public  Stream CameraTake() {
//            try
//            {
//                var imageProperties = ImageEncodingProperties.CreateJpeg();
//                var stream = new InMemoryRandomAccessStream();
//
//                ((Task) _mediaCapture.CapturePhotoToStreamAsync(imageProperties, stream)).Wait();
//
//                Debug.WriteLine("Take Photo succeeded.");
//
//                CameraStream = stream.AsStreamForRead();
//                return CameraStream;
//
//            }
//            catch (Exception ex)
//            {
//                Debug.Write("Exception: " + ex.Message);
//                CameraCleanup();
//                return null;
//            }
//        }



    }


    public sealed class HttpServer : IDisposable
    {
        private const uint BufferSize = 8192;
        private readonly int port = 8000;
        private StreamSocketListener listener;

        private readonly MainPage _main;

        public HttpServer(int serverPort, MainPage main)
        {
            listener = new StreamSocketListener();
            listener.Control.KeepAlive = true;
            listener.Control.NoDelay = true;

            _main = main;

            port = serverPort;
            listener.ConnectionReceived += async (s, e) =>
            {
                await ProcessRequestAsync(e.Socket);
                try
                {
                    e.Socket.Dispose();
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                }
            };
        }

        public void StartServer()
        {
            Task.Run(async () =>
            {
                await listener.BindServiceNameAsync(port.ToString());
            });
        }


        public void Dispose()
        {
            listener.Dispose();
        }

        private async Task ProcessRequestAsync(StreamSocket socket)
        {
            // this works for text only
            StringBuilder request = new StringBuilder();
            byte[] data = new byte[BufferSize];
            IBuffer buffer = data.AsBuffer();
            uint dataRead = BufferSize;
            using (IInputStream input = socket.InputStream)
            {
                while (dataRead == BufferSize)
                {
                    await input.ReadAsync(buffer, BufferSize, InputStreamOptions.Partial);
                    request.Append(Encoding.UTF8.GetString(data, 0, data.Length));
                    dataRead = buffer.Length;
                }
            }

            string requestAsString = request.ToString();
            string[] splitRequestAsString = requestAsString.Split('\n');
            if (splitRequestAsString.Length != 0)
            {
                string requestMethod = splitRequestAsString[0];
                string[] requestParts = requestMethod.Split(' ');
                if (requestParts.Length > 1)
                {
                    if (requestParts[0] == "GET")
                    {
                        try
                        {
                            WriteResponse(requestParts[1], socket);
                        }
                        catch (Exception ex)
                        {


                            using (var outputStream = socket.OutputStream)
                            {
                                using (Stream resp = outputStream.AsStreamForWrite())
                                {
                                    byte[] bodyArray = Encoding.UTF8.GetBytes("Caught Exception: " + ex.Message);
                                    using (MemoryStream stream = new MemoryStream(bodyArray))
                                    {
                                        string header = String.Format("HTTP/1.1 500 Server Error\r\n" +
                                                                      "Content-Length: {0}\r\n" +
                                                                      "Connection: close\r\n\r\n",
                                            stream.Length);
                                        byte[] headerArray = Encoding.UTF8.GetBytes(header);
                                        resp.Write(headerArray, 0, headerArray.Length);
                                        stream.CopyTo(resp);
                                        await resp.FlushAsync();
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        using (var outputStream = socket.OutputStream)
                        {
                            using (Stream resp = outputStream.AsStreamForWrite())
                            {
                                byte[] bodyArray = Encoding.UTF8.GetBytes("Method Not Allowed: " + requestParts[0]);
                                using (MemoryStream stream = new MemoryStream(bodyArray))
                                {
                                    string header = String.Format("HTTP/1.1 405 Method Not Allowed\r\n" +
                                                                  "Content-Length: {0}\r\n" +
                                                                  "Connection: close\r\n\r\n",
                                        stream.Length);
                                    byte[] headerArray = Encoding.UTF8.GetBytes(header);
                                    resp.Write(headerArray, 0, headerArray.Length);
                                    stream.CopyTo(resp);
                                    await resp.FlushAsync();
                                }
                            }
                        }
                    }
                }
            }
        }

        private void WriteResponse(string request, StreamSocket socket)
        {
            try
            {
                Debug.WriteLine("New Request: " + request);

                string[] splitRequest = request.Split("?".ToCharArray(), 2);
                string target = splitRequest[0];
                if (target == "/fly" || target == "/fly/")
                {
                    if (splitRequest.Length > 1)
                    {
                        string[] valueStr = splitRequest[1].Split(',');
                        for (int key = 0; key < valueStr.Length; key++)
                        {
                            int value = Int32.Parse(valueStr[key], System.Globalization.NumberStyles.HexNumber);
                            _main.SetPWM(key, value);
                        }
                    }


                    byte[] bodyArray = Encoding.UTF8.GetBytes(_main.getPwmString());
                    // Show the html 
                    using (var outputStream = socket.OutputStream)
                    {
                        using (Stream resp = outputStream.AsStreamForWrite())
                        {
                            using (MemoryStream stream = new MemoryStream(bodyArray))
                            {
                                string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                                    "Content-Length: {0}\r\n" +
                                                    "Connection: close\r\n\r\n",
                                                    stream.Length);
                                byte[] headerArray = Encoding.UTF8.GetBytes(header);
                                resp.Write(headerArray, 0, headerArray.Length);
                                stream.CopyTo(resp);
                                resp.Flush();
                            }
                        }
                    }



                }
                else if (target == "/image" || target == "/image/")
                {
                    if (splitRequest.Length > 1)
                    {
                        if (int.Parse(splitRequest[1]) == 1)
                        {
                            // Show the image 
                            using (var outputStream = socket.OutputStream)
                            {

                                _main.CameraTake();

//                                if (stream == null)
//                                    throw new NullReferenceException("The stream with the image is null.");
//
//                                stream.Seek(0, SeekOrigin.Begin);


                                using (Stream resp = outputStream.AsStreamForWrite())
                                {

                                    //                                var fileStream = File.Create("C:\\Path\\To\\File");
                                    //                                myOtherObject.InputStream.Seek(0, SeekOrigin.Begin);
                                    //                                myOtherObject.InputStream.CopyTo(fileStream);
                                    //                                fileStream.Close();

                                    

                                    string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                                    "Content-Length: {0}\r\n" +
                                                    "Connection: Keep-Alive\r\n",
                                                    "Content-Type: image/png\r\n\r\n",
                                                    _main.CameraStream.Length);
                                    byte[] headerArray = Encoding.UTF8.GetBytes(header);
                                    resp.Write(headerArray, 0, headerArray.Length);
                                    _main.CameraStream.CopyTo(resp);

                                    using (
                                        FileStream fsSource = new FileStream(_main.PHOTO_FILE_NAME, FileMode.Open,
                                            FileAccess.Read))
                                    {
                                        fsSource.CopyTo(resp);
                                    }
                                }
                                ((Task)outputStream.FlushAsync()).Wait();
                            }
                        }
                        else if (int.Parse(splitRequest[1]) == 2)
                        {
                        
                        }
                    }
                }
                else
                {
                    Debug.WriteLine(target);

                    byte[] bodyArray = Encoding.UTF8.GetBytes("Not Found");
                    using (var outputStream = socket.OutputStream)
                    {
                        using (Stream resp = outputStream.AsStreamForWrite())
                        {
                            using (MemoryStream stream = new MemoryStream(bodyArray))
                            {
                                string header = String.Format("HTTP/1.1 404 Not Found\r\n" +
                                                    "Content-Length: {0}\r\n" +
                                                    "Connection: close\r\n\r\n",
                                                    stream.Length);
                                byte[] headerArray = Encoding.UTF8.GetBytes(header);
                                resp.Write(headerArray, 0, headerArray.Length);
                                stream.CopyTo(resp);
                                resp.Flush();
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Exception: " + ex.Message);
            }
        }
    }
}
