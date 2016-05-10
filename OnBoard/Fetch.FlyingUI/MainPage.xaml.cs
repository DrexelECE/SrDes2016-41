using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Media.Effects;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;
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
        private PwmController hat;

        public MainPage()
        {
            Debug.WriteLine("Package Family Name:\n  " + Package.Current.Id.FamilyName + "\n");

            this.InitializeComponent();

            // init the hat
            hat = new PwmController();
            hat.SetDesiredFrequency(364); // value determined experimentally.  See notebook for targets in Hz. 


            // init listening for app commiuncation
            ApplicationData.Current.DataChanged += async (d, a) => await HandleDataChangedEvent(d, a);

            while (true)
            {

                hat.SetPulseParameters(15, 2000);
                Task.Delay(TimeSpan.FromSeconds(5)).Wait();
            }
        }

        private async Task HandleDataChangedEvent(ApplicationData data, object args)
        {
            try
            {
                var localSettings = ApplicationData.Current.LocalSettings;
                if (!localSettings.Values.ContainsKey("pwmState"))
                {
                    return;
                }

                ushort? newState = localSettings.Values["pwmState"] as ushort?;

                switch (newState)
                {
                    case null:
                        return;

                    default:
                        await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                        {
                            hat.SetPulseParameters(15, 1000);
                        });
                        break;
                }

            }
            catch (Exception ex)
            {
                // Do nothing meaningful.  Mostly for lack of anything better to do. 
                Debug.WriteLine(ex.ToString());
            }
        }
    }
}
