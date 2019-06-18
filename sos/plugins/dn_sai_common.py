from sos.plugins import Plugin, DebianPlugin
import os

class DN_sai_commonPlugin(Plugin, DebianPlugin):
    """ Collects SAI debugging information
    """

    plugin_name = os.path.splitext(os.path.basename(__file__))[0]
    profiles = ('networking', 'dn', 'sai')

    def setup(self):
        self.add_cmd_output("ifconfig -a")
        self.add_cmd_output("ip link show")
        tmp = os.popen("ps -Af").read()
        if "hshell" not in tmp[:]:
            self.add_cmd_output("/opx/bin/opx-switch-shell ps", timeout = 10)
            self.add_cmd_output("/opx/bin/opx-switch-shell 'phy info'", timeout = 10)
        else:
            self.add_alert("hshell is already running! halt hshell and rerun sosreport!")
        self.add_copy_spec("/etc/opx/sai/init.xml")
