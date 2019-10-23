using System;
using System.Collections.Generic;
using System.Linq;
using zhinst;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace LabOne
{

  [TestClass]
  public class Examples
  {
    // device name needs to be adjusted to match the device in use
    static String dev = "dev574";
    [ClassInitialize]
    public static void ClassSetup(TestContext a)
    {
      if (a.Properties.Contains("device"))
      {
        dev = a.Properties["device"].ToString();
      }
      System.Diagnostics.Trace.WriteLine("Using device " + dev);
    }

    // The resetDeviceToDefault will reset the device settings
    // to factory default. The call is quite expensive
    // in runtime. Never use it inside loops!
    private void resetDeviceToDefault(ziDotNET daq)
    {
      // The HF2 devices do not support the preset functionality.
      if (isDeviceFamily(daq, "HF2"))
      {
        daq.setDouble(String.Format("/{0}/demods/*/rate", dev), 250);
      }
      else
      {
        daq.setInt(String.Format("/{0}/system/preset/index", dev), 0);
        daq.setInt(String.Format("/{0}/system/preset/load", dev), 1);
        while (daq.getInt(String.Format("/{0}/system/preset/busy", dev)) != 0) {
          System.Threading.Thread.Sleep(100);
        }
        System.Threading.Thread.Sleep(1000);
      }
    }

    // The isDeviceFamily checks for a specific device family.
    // Currently available families: "HF2", "UHF", "MF"
    private bool isDeviceFamily(ziDotNET daq, String family)
    {
      String path = String.Format("/{0}/features/devtype", dev);
      String devType = daq.getByte(path);
      return devType.StartsWith(family);
    }

    // The hasOption function checks if the device
    // does support a specific functionality, thus
    // has installed the option.
    private bool hasOption(ziDotNET daq, String option)
    {
      String path = String.Format("/{0}/features/options", dev);
      String options = daq.getByte(path);
      return options.Contains(option);
    }

    // Please handle version mismatches depending on your
    // application requirements. Version mismatches often relate
    // to functionality changes of some nodes. The API interface is still
    // identical. We strongly recommend to keep the version of the
    // API and data server identical. Following approaches are possible:
    // - Convert version mismatch to a warning for the user to upgrade / downgrade
    // - Convert version mismatch to an error to enforce full matching
    // - Do an automatic upgrade / downgrade
    private void apiServerVersionCheck(ziDotNET daq)
    {
      String serverVersion = daq.getByte("/zi/about/version");
      String apiVersion = daq.version();

      Assert.AreEqual(serverVersion, apiVersion,
        "Version mismatch between LabOne API and Data Server.");
    }

    // Connect initializes a session on the server.
    private ziDotNET connect()
    {
      ziDotNET daq = new ziDotNET();
      String id = daq.discoveryFind(dev);
      String iface = daq.discoveryGetValueS(dev, "connected");
      //if (string.IsNullOrWhiteSpace(iface))
      if(string.IsNullOrEmpty(iface))
      {
        // Device is not connected to the server
        String ifacesList = daq.discoveryGetValueS(dev, "interfaces");
        // Select the first available interface and use it to connect
        string[] ifaces = ifacesList.Split('\n');
        if (ifaces.Length > 0)
        {
          iface = ifaces[0];
        }
      }
      String host = daq.discoveryGetValueS(dev, "serveraddress");
      long port = daq.discoveryGetValueI(dev, "serverport");
      long api = daq.discoveryGetValueI(dev, "apilevel");
      System.Diagnostics.Trace.WriteLine(
        String.Format("Connecting to server {0}:{1} wich API level {2}",
        host, port, api));
      daq.init(host, Convert.ToUInt16(port), (ZIAPIVersion_enum)api);
      // Ensure that LabOne API and LabOne Data Server are from
      // the same release version.
      apiServerVersionCheck(daq);
      // If device is not yet connected a reconnect
      // will not harm.
      System.Diagnostics.Trace.WriteLine(
        String.Format("Connecting to {0} on inteface {1}", dev, iface));
      daq.connectDevice(dev, iface, "");

      return daq;
    }

    // ExamplePollDemodSample connects to the device,
    // subscribes to a demodulator, polls the data for 0.1 s
    // and returns the data.
    
    public void ExamplePIDsetloc(double setpoint)
    {
      ziDotNET daq = connect();
      if (!hasOption(daq, "PID"))
      {
        daq.disconnect();
                Console.WriteLine("No PID option");
        return;
      }

     
      if (isDeviceFamily(daq, "HF2"))
      {
        Console.WriteLine("DeviceFamily");
        String path = String.Format("/{0}/pids/0/setpoint", dev);

        double setpt = daq.getDouble(path);
        Console.WriteLine("Old stepoint is {0}\n",setpt);
        daq.setDouble(String.Format("/{0}/pids/0/setpoint", dev), setpoint);
        /*daq.setInt(String.Format("/{0}/pids/0/output", dev), pid_output);
        daq.setInt(String.Format("/{0}/pids/0/outputchannel", dev), pid_output_channel);
        daq.setDouble(String.Format("/{0}/pids/0/center", dev), pid_center_frequency);
        daq.setInt(String.Format("/{0}/pids/0/enable", dev), 0);
        daq.setInt(String.Format("/{0}/pids/0/phaseunwrap", dev), phase_unwrap);
        daq.setDouble(String.Format("/{0}/pids/0/limitlower", dev), -pid_limits);
        daq.setDouble(String.Format("/{0}/pids/0/limitupper", dev), pid_limits);
        */
        setpt = daq.getDouble(path);
        Console.WriteLine("new stepoint is {0}\n", setpt);
      }
      // Perform a global synchronisation between the device and the data server:
      // Ensure that the settings have taken effect on the device before starting
      // the pidAdvisor.
      
    }
  }
}
