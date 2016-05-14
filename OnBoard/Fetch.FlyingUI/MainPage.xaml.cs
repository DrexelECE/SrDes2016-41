using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
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
                int inc = 128;
                int lev = 3072;
                
                while (true)
                {
                    if (lev < 2048 || lev > 4095)
                        inc = -inc;
                
                    lev += inc;
                
                    SetPWM(15, lev);
                    Task.Delay(TimeSpan.FromMilliseconds(20)).Wait();
                }
            });



        }

        public void SetPWM(int which, int setTo)
        {
            setTo = (setTo > 4094 ? 4094 : setTo);
            setTo = (setTo < 1 ? 1 : setTo);

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

                PreviewElement.Source = _mediaCapture;
                await _mediaCapture.StartPreviewAsync();

                Debug.WriteLine("done.");

                IReadOnlyList<IMediaEncodingProperties> resolutions = _mediaCapture.VideoDeviceController.GetAvailableMediaStreamProperties(MediaStreamType.VideoPreview);




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

        public async Task<Stream> CameraTake() {
            try
            {
                ImageEncodingProperties imageProperties = ImageEncodingProperties.CreateJpeg();
                InMemoryRandomAccessStream stream = new InMemoryRandomAccessStream();

                await _mediaCapture.CapturePhotoToStreamAsync(imageProperties, stream);

                Debug.Write("Take Photo succeeded.");

                return stream.AsStreamForRead();

            }
            catch (Exception ex)
            {
                Debug.Write("Exception: " + ex.Message);
                CameraCleanup();
                return null;
            }
        }



    }


    public sealed class HttpServer : IDisposable
    {
        private const string offHtmlString = "<html><head><title>Blinky App</title></head><body><form action=\"blinky.html\" method=\"GET\"><input type=\"radio\" name=\"state\" value=\"on\" onclick=\"this.form.submit()\"> On<br><input type=\"radio\" name=\"state\" value=\"off\" checked onclick=\"this.form.submit()\"> Off</form></body></html>";
        private const string onHtmlString = "<html><head><title>Blinky App</title></head><body><form action=\"blinky.html\" method=\"GET\"><input type=\"radio\" name=\"state\" value=\"on\" checked onclick=\"this.form.submit()\"> On<br><input type=\"radio\" name=\"state\" value=\"off\" onclick=\"this.form.submit()\"> Off</form></body></html>";
        private const uint BufferSize = 8192;
        private int port = 8000;
        private StreamSocketListener listener;

        private MainPage _main;

        public HttpServer(int serverPort, MainPage main)
        {
            listener = new StreamSocketListener();
            listener.Control.KeepAlive = true;
            listener.Control.NoDelay = true;

            _main = main;

            port = serverPort;
            listener.ConnectionReceived += async (s, e) => { await ProcessRequestAsync(e.Socket); };
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
                        WriteResponse(requestParts[1], socket);
                    else
                        throw new InvalidDataException("HTTP method not supported: "
                            + requestParts[0]); // TODO look into returning a 400-class error, instead of throwing an excption. 
                }
            }
        }

        private async void WriteResponse(string request, StreamSocket socket)
        {
            // See if the request is for blinky.html, if yes get the new state
            string state = "Unspecified";
            string[] splitRequest = request.Split("?".ToCharArray(), 2);
            string target = splitRequest[0];
            if (target == "/fly" || target == "/fly/")
            {
                if (splitRequest.Length > 1)
                {
                    string[] valueStr = splitRequest[1].Split(',');
                    for (int key = 0; key < valueStr.Length; ++key)
                    {
                        int value = Int32.Parse(valueStr[key], System.Globalization.NumberStyles.HexNumber);
                        _main.SetPWM(key, value);
                    }
                }


                // TODO replace output with something more useful for the task at hand. 
                string html = state == "On" ? onHtmlString : offHtmlString;
                byte[] bodyArray = Encoding.UTF8.GetBytes(html);
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
            else if (target == "/image" || target == "/image")
            {
                if (splitRequest.Length > 1)
                {
                    if (Int32.Parse(splitRequest[1]) == 1)
                    {
                        //TODO rearrange. 
                    }

                    // Show the image 
                    using (var outputStream = socket.OutputStream)
                    {
                        using (Stream resp = outputStream.AsStreamForWrite())
                        {

                                Stream stream = await _main.CameraTake();

                                string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                                    "Content-Length: {0}\r\n" +
                                                    "Content-Type: image/jpeg\r\n" +
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
            else
            {
                string html = state == "On" ? onHtmlString : offHtmlString;
                byte[] bodyArray = Encoding.UTF8.GetBytes(html);
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
        }
    }
}
