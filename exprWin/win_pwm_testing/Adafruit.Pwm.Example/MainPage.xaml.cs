using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
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

        private void RotateExample()
        {
            try
            {
                //The servoMin/servoMax values are dependant on the hardware you are using.
                //The values below are for my SR-4303R continuous rotating servos.
                //If you are working with a non-continous rotatng server, it will have an explicit
                //minimum and maximum range; crossing that range can cause the servo to attempt to
                //spin beyond its capability, possibly damaging the gears.

                const int servoMin = 300;  // Min pulse length out of 4095
                const int servoMax = 480;  // Max pulse length out of 4095

                using (var hat = new Adafruit.Pwm.PwmController())
                {
                    hat.SetDesiredFrequency(500); // spec'd to 658?



                    while (true)
                    {
                        //hat.SetPulseParameters(4, 4015); //aux
                        //hat.SetPulseParameters(3, 4015); //rud
                        //hat.SetPulseParameters(0, 160); //thr

                        //Task.Delay(TimeSpan.FromSeconds(5)).Wait();

                        //// 10 -> 160
                        //// 128 -> 2055
                        //// 250 -> 4015

                        //hat.SetPulseParameters(4, 160); //aux
                        //hat.SetPulseParameters(3, 2055); //rud
                        //hat.SetPulseParameters(2, 2055); //ele
                        //hat.SetPulseParameters(1, 2055); //ail
                        //hat.SetPulseParameters(0, 4015); //thr

                        //Task.Delay(TimeSpan.FromSeconds(5)).Wait();

                        //hat.SetPulseParameters(0, 160); //thr

                        //Task.Delay(TimeSpan.FromSeconds(10)).Wait();

                        hat.SetPulseParameters(4, 40); //aux
                        hat.SetPulseParameters(3, 40); //rud
                        hat.SetPulseParameters(2, 40); //ele
                        hat.SetPulseParameters(1, 40); //ail
                        hat.SetPulseParameters(0, 40); //thr

                        Task.Delay(TimeSpan.FromSeconds(5)).Wait();

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
