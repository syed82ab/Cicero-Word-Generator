namespace AtticusServer
{
    using System;
    using System.Runtime.InteropServices;
    

    public class ziHF : object
    {

        //public struct Atticus ZIconnection
        //{ public IntPtr conn;
        //}
        //public struct ZIDoubleData
        //{
        //    public double value;

        //    //public static explicit operator ZIDoubleData(string v)
        //    //{
        //    //  value = Convert.ToDouble(v);
        //    //}
        //}
        IntPtr ZIconn = new IntPtr();

        private bool _disposed = true;

        ~ziHF() { Dispose(false); }

      
        public ziHF(ref IntPtr ZIconn)
        {
            int pInvokeResult = PInvoke.Init(ref ZIconn);
            //PInvoke.TestForError(this._handle, pInvokeResult);
            this._disposed = false;
        }


        public int Connect(IntPtr ZIconn)
        {
            int pInvokeResult = PInvoke.Connect(ZIconn);
            //PInvoke.TestForError(this._handle, pInvokeResult);
            return pInvokeResult;
        }
        public int SetD(IntPtr ZIconn , double value)
        {
            int pInvokeResult = PInvoke.SetD(ZIconn, value);
            //PInvoke.TestForError(this._handle, pInvokeResult);
            return pInvokeResult;
        }
        public int SyncSetD(IntPtr ZIconn, double value)
        {
            int pInvokeResult = PInvoke.SyncSetD(ZIconn, value);
            //PInvoke.TestForError(this._handle, pInvokeResult);
            return pInvokeResult;
        }
        public int TogglePID1(IntPtr ZIconn, int value)
        {
            int pInvokeResult = PInvoke.TogglePID1(ZIconn, value);
            return pInvokeResult;

        }
        public int GetPID1(IntPtr ZIconn, double value)
        {
            int pInvokeResult = PInvoke.GetPID1(ZIconn, value);
            return pInvokeResult;
        }
        /*
       public int EnablePID1(IntPtr ZIconn)
        {
            int pInvokeResult = PInvoke.EnablePID1(ZIconn);
            return pInvokeResult;

        }*/
        public void Disconnect(IntPtr ZIconn)
        {
            PInvoke.Disconnect(ZIconn);
            //PInvoke.TestForError(this._handle, pInvokeResult);
            //return pInvokeResult;
        }
        public void Destroy(IntPtr ZIconn)
        {
            PInvoke.Destroy(ZIconn);
            //PInvoke.TestForError(this._handle, pInvokeResult);
            //return pInvokeResult;
        }

        public void Dispose()
        {
            this.Dispose(true);
            System.GC.SuppressFinalize(this);
        }
        private void Dispose(bool disposing)
        {
            if ((this._disposed == false))
            {
                PInvoke.Destroy(this.ZIconn);
                this.ZIconn = System.IntPtr.Zero;
            }
            this._disposed = true;
        }
      
      
        private class PInvoke
        {

            [DllImport("setpointPID1.dll", EntryPoint = "ziInit", CallingConvention = CallingConvention.StdCall)]
            public static extern int Init(ref IntPtr conn);

            [DllImport("setpointPID1.dll", EntryPoint = "ziConnect", CallingConvention = CallingConvention.StdCall)]
            public static extern int Connect(IntPtr conn);

            [DllImport("setpointPID1.dll", EntryPoint = "ziSetValueD", CallingConvention = CallingConvention.StdCall)]
            public static extern int SetD(IntPtr conn, double value);

            [DllImport("setpointPID1.dll", EntryPoint = "ziGetPID1", CallingConvention = CallingConvention.StdCall)]
            public static extern int GetPID1(IntPtr conn, double value);


            [DllImport("setpointPID1.dll", EntryPoint = "ziSyncSetValueD", CallingConvention = CallingConvention.StdCall)]
            public static extern int SyncSetD(IntPtr conn, double value);

            [DllImport("setpointPID1.dll", EntryPoint = "ziTogglePID1", CallingConvention = CallingConvention.StdCall)]
            public static extern int TogglePID1(IntPtr conn, int value);
          
            [DllImport("setpointPID1.dll", EntryPoint = "ziDisconnect", CallingConvention = CallingConvention.StdCall)]
            public static extern void Disconnect(IntPtr conn);
            [DllImport("setpointPID1.dll", EntryPoint = "ziDestroy", CallingConvention = CallingConvention.StdCall)]
            public static extern void Destroy(IntPtr conn);
            
      

        }   
    }
}
