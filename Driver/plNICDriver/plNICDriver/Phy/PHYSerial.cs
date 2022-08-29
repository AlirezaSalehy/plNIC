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
		private Mutex txMutex = new Mutex();
		private Mutex rxMutex = new Mutex();
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

		public IBasicPhy.Status SendBytes(byte[] bytes, int offset, int numBytes)
		{
			txMutex.WaitOne();
			_serialPort.WriteTimeout = int.MaxValue;
			try
			{
				_serialPort.Write(bytes, offset, numBytes);
				return IBasicPhy.Status.Success;

			}
			catch (Exception ex)
			{
				Console.WriteLine("PHY Trasmit EXCP: " +  ex.Message);
				return IBasicPhy.Status.Failure;
			} finally
			{
				txMutex.ReleaseMutex();
			}
		}

		public IBasicPhy.Status ReceiveBytes(byte[] bytes, int offset, int numBytes)
		{
			rxMutex.WaitOne();
			_serialPort.ReadTimeout = 500;
			int numRecv = 0;
			while (0 < numBytes)
			{
				try
				{
					numRecv += _serialPort.Read(bytes, numRecv+offset, numBytes);
					numBytes -= numRecv;
				} catch { }
			}
			rxMutex.ReleaseMutex();
			return IBasicPhy.Status.Success;
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
