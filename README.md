## Travel card reader for Sailfish OS

**IMPORTANT NOTE!** Roughly since November 2025, Helsinki public transport
validators stopped updating the information stored on the cards. Meaning that
this app can no longer read anything useful from the HSL cards (not even the
balance) because there's nothing useful there to read. Everything these days
is done over the Internet (when it works) and stored somewhere on some server
in some sort of a database. Your data don't belong to you anymore.

![icon](icons/86x86/harbour-matkakortti.png)

Application for reading [Helsinki](https://www.hsl.fi/en/changecard) and
[Tampere](https://joukkoliikenne.tampere.fi/liput-ja-hinnat/tampereen-matkakortti.html)
travel cards. The information describing NFC API and data format for HSL cards and
even some documentation can be found [here](https://github.com/HSLdevcom/hsl-card-java)
(in Finnish).

Obviously, this application only works on those phones where NFC is
[supported](https://jolla.com/sailfishx) by Sailfish OS, e.g. Sony Xperia XA2
or Xperia 10. Support for **Tampere** travel cards requires **Sailfish OS 3.3.0**
or later - earlier versions of Sailfish OS handled Tampere cards in such a way
that made those cards unhappy and unwilling to work.

<img alt="HSL card" src="qml/hsl/images/hsl-card.svg" width="254">
<img alt="Tampere card" src="qml/nysse/images/nysse-card.svg" width="254">
