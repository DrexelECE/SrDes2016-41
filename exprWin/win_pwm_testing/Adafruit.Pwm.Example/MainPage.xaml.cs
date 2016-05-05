using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.ServiceModel;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Adafruit;


namespace Adafruit.Pwm.Example
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        public MainPage()
        {
            this.InitializeComponent();
            RotateExample();
            Application.Current.Exit();
        }

        [WebMethod]
        public void set()

        private void RotateExample()
        {
            try
            {

                using (var hat = new PwmController())
                {
                    hat.SetDesiredFrequency(364); // spec'd to 658?

                    while (true)
                    {

                        int s = 0;
                        int m = 8;

                        int max = (int) ((float) 4096*(0.75));
                        int min = (int) ((float) 4096*(0.28));

                        while (true)
                        {
                            s += m;
                            if (s >= max)
                            {
                                s = max;
                                m = -4;
                            }
                            if (s <= min)
                            {
                                s = min;
                                m = 4;
                            }
                            hat.SetPulseParameters(15, s);

                            Task.Delay(TimeSpan.FromMilliseconds(10)).Wait();
                        }

                        hat.SetPulseParameters(15, 0);

                        hat.SetPulseParameters(4, 40); //aux
                        hat.SetPulseParameters(3, 40); //rud
                        hat.SetPulseParameters(2, 40); //ele
                        hat.SetPulseParameters(1, 40); //ail
                        hat.SetPulseParameters(0, 40); //thr

                        Task.Delay(TimeSpan.FromSeconds(5)).Wait();

                        hat.SetPulseParameters(15, 4096);

                        hat.SetPulseParameters(4, 4055); //aux
                        hat.SetPulseParameters(3, 4055); //rud
                        hat.SetPulseParameters(2, 4055); //ele
                        hat.SetPulseParameters(1, 4055); //ail
                        hat.SetPulseParameters(0, 4055); //thr

                        Task.Delay(TimeSpan.FromSeconds(5)).Wait();
                    }
                }
            }

            /* If the write fails display the error and stop running */
            catch (Exception ex)
            {
                Text_Status.Text = "Failed to communicate with device: " + ex.Message;
                return;
            }

        }
    }
}
