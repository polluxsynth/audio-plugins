Audio plugins
=============

This repository contains my Distrho Plugin Framework based audio plugins.
(Well, that sounds rather grand; currently there is but one).

Contains the following plugin:

* [MiMi-d](https://butoba.net/homepage/mimid.html),
  a digital rendering of the [MiMi-a](https://butoba.net/homepage/mimia.html)
  microprocessor controlled analog polyphonic synthesizer.
  It is mainly intended as an instrument for the Zynthian platform, and as
  such it does not contain any UI of its own, and relies on the host to provide
  one. 

  Although technically not platform dependent, for now this is mainly intended
  to be run on the [Zynthian](https://zynthian.org/) platform. Because
  of that, there is no GUI, and the plugin relies on the host to provide
  a GUI and also patch storage and retrieval functions.

To build, run `make` (this also fetches the DPF framework if it
has not been done already), followed by `make install` with the appropriate
privileges, to install the plugin bundle(s) in `/usr/local/lib/lv2`. 
This installs both the plugin itself in the appropriate directory, as
well as any supplied factory presets in a parallel directory.

Enabling MiMi-d on the Zynthian platform
----------------------------------------

MiMi-d has been included in the Zynthian platform for a while now,
enabled by default, so it's just a question of dialing up **MiMi-d**
among the list of synthesizer plugins when adding a new chain.

If you're using an older Zynthian version, or MiMi-d for some
reason isn't enabled, do the following:

On Zynthian, after building, go to the web configuration, select
the **Software** menu, then **LV2-plugins**, and then click on the wide
**Search for new plugins and presets** button. 
This operation will take some time, often several minutes, during which
the button will be red (while the page attempts to reload). When it
has completed (the button reverts to black) **MiMi-d** can be enabled
in the list of plugins. Finally click **Save**.
It seems that sometimes a reboot of the Zynthian is needed as well.

The same procedure is used when updating with a new MiMi-d release, except
that the plugin will already have been enabled in the webconf plugin
list, so it's just a question of rebuilding MiMi-d, installing it,
and rebooting Zynthian.
