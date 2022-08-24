using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Phy
{
	public class PHYSerial : IBasicPhy
	{
		private SerialPort _serialPort;

		public static string[] GetAvailablePortsName()
		{
			return SerialPort.GetPortNames();
		}

		public PHYSerial(string portName)
		{
			_serialPort = new SerialPort()
			{
				PortName = portName,
				BaudRate = 115200,
				Parity = Parity.None,
				StopBits = StopBits.One,	
				DataBits = 8,
				Handshake = Handshake.None,
				ReadTimeout = 500,
				WriteTimeout = 500
			};

			_serialPort.Open();
		}


		public IBasicPhy.Status SendBytes(byte[] bytes, int numBytes, int timeOutMillis)
		{
			_serialPort.WriteTimeout = timeOutMillis;
			try
			{
				_serialPort.Write(bytes, 0, numBytes);
				return IBasicPhy.Status.Success;

			}
			catch (Exception ex)
			{
				Console.WriteLine(ex.Message);
				return IBasicPhy.Status.Failure;
			}
		}

		public IBasicPhy.Status ReceiveBytes(byte[] bytes, int numBytes, int timeOutMillis)
		{
			_serialPort.ReadTimeout = timeOutMillis;
			try
			{
				_serialPort.Read(bytes, 0, numBytes);
				return IBasicPhy.Status.Success;
			} catch (Exception ex) {
				Console.WriteLine(ex.Message);
				return IBasicPhy.Status.Failure;
			}
		}

		public void Dispose()
		{
			_serialPort.Close();
			_serialPort.Dispose();
		}
	}
}
