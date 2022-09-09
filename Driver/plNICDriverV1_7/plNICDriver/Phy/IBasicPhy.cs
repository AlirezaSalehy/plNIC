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
		public delegate void OnBytesRx(byte[] bytes);
		public Task<Status> SendBytes(byte[] bytes, int offset, int numBytes);
	}
}
