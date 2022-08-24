using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Phy
{
	public interface IBasicPhy : IDisposable
	{
		public enum Status
		{
			Success,
			Failure
		};

		public Status SendBytes(byte[] bytes, int numBytes, int timeOutMillis);
		public Status ReceiveBytes(byte[] bytes, int numBytes, int timeOutMillis);
	}
}
