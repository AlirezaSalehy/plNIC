using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static plNICDriver.Phy.IBasicPhy;

namespace plNICDriver.Phy
{
	public class PHYSerial : IBasicPhy
	{
		private Mutex txMutex = new Mutex();
		private Mutex rxMutex = new Mutex();
		private SerialPort _serialPort;
		private OnBytesRx _onRx;

		public static string[] GetAvailablePortsName()
		{
			return SerialPort.GetPortNames();
		}

		public PHYSerial(string portName, OnBytesRx onRx)
		{
			_onRx = onRx;
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

			_serialPort.DataReceived += DataReceivedHandler;
			_serialPort.Open();
		}

		public async Task<Status> SendBytes(byte[] bytes, int offset, int numBytes)
		{
			try
			{
				txMutex.WaitOne();
				_serialPort.WriteTimeout = int.MaxValue;
				_serialPort.Write(bytes, offset, numBytes);
				return Status.Success;
			}
			catch (Exception ex)
			{
				Console.WriteLine("PHY Trasmit EXCP: " +  ex.Message);
				return Status.Failure;
			} finally
			{
				txMutex.ReleaseMutex();
				await Task.Delay((numBytes) * 100);
			}
		}

		private void DataReceivedHandler(
					  object sender,
					  SerialDataReceivedEventArgs e)
		{
			var avail = _serialPort.BytesToRead;
			var bytes = new byte[avail];
			_serialPort.Read(bytes, 0, avail);
			_onRx(bytes);
		}

		public void Dispose()
		{
			rxMutex.WaitOne(5000);
			txMutex.WaitOne(5000);
			rxMutex.Close();
			txMutex.Close();
			_serialPort.Close();
			_serialPort.Dispose();
		}
	}
}
