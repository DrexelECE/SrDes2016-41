using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

using System.Threading.Tasks;
using Windows.UI.ViewManagement;
using Adafruit.Pwm;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace OnBoard
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        
        private Adafruit.Pwm.PwmController _hat;
        private ushort[] _pwmValues = new ushort[16];

        public MainPage()
        {

            // Graphical things
            InitializeComponent();


            // Init hat
            InitHat();

            Application.Current.Exit();
        }



        /// <summary>
        /// Initialization for the PWM Hat. 
        /// </summary>
        private void InitHat()
        {
            _hat = new PwmController();

            // see notebook for frequency targeting.  This value was found experimentally.
            _hat?.SetDesiredFrequency(364);
        }



        /// <summary>
        /// Intrface for setting PWM value on a given pin. 
        /// </summary>
        /// <param name="which">Pin number.  Starts at 0.</param>
        /// <param name="level">Level to which value should be set, along the full ushort range.</param>
        /// <returns>False when arguments are invalid.</returns>
        public bool SetPwm(int which, ushort level)
        {
            try
            {
                if (which < 0 || which >= _pwmValues.Length)
                    return false;

                _pwmValues[which] = level;
                _SetPwmPin(which, level);
                return true;
            }
            catch (Exception ex)
            {
                Debug.Write(ex.ToString());
                return false;
            }
        }


        /// <summary>
        /// Abstraction for the Adafruit PWM call.  
        /// 
        /// Includes conversion from ushort to format needed for lib. 
        /// </summary>
        /// <param name="which">The number of the pin.  Starts at 0</param>
        /// <param name="level">The level to which the pin should be set. </param>
        private void _SetPwmPin(int which, ushort level)
        {
            _hat.SetPulseParameters(which, level * (4095 / ushort.MaxValue));
        }

    }
}
