

/*Programowanie urządzeń USB to zasadniczo skomplikowany proces. 
Wymaga wiedzy jak pisać deskryptory dla poszczególnych klas, 
jak je budować i jak stworzyć deskryptor konfiguracji. Wiadomo najlepiej jest spojrzeć na 
gotowe implementacje w tym celu. Po przeczytaniu kilku krotnym deskryptorów dla danej klasy można 
się podjąć próby złożenia dwóch klas w urządzenie kompozytowe. Wtedy uzyskujemy urządzenie 
złożone z dwóch klas np. Mysz(HID) + Port COM(CDC). Ponadto potrzebna jest wiedza jak skorzystać 
z implementacji sterownika czyli warstwy abstrakcji lub wymagana jest świadomość działania protokołu 
USB aby móc z niej skorzystać. Aby w pełni zrozumieć działanie sprzętu oraz mieć nad nim kontrole
najlepiej jest w pełni przeczytać i zrozumieć wybraną przykładową implementacje sterownika lub spróbować 
samodzielnie taki sterownik przygotować, co jest raczej skomplikowanym i czasochłonnym zadaniem.
*/

/*
  * Odradzam serdecznie uczyc sie USB :) Strata czasu :( 
  * Szkoda ze juz za pozno, zbyt skomplikowane do efektu jaki uzyskacie
  * mnostwo rejestrow kolejki..
  * zadania, maszyna stanowa realizujaca transmisje danych sterujacych...
  * pakiety in out...
  * deksryptory? Deksryptory kuwa...
  * EndPointy? Wez je skonfiguruj poprawnie...
  * Rodzaje transmisji: control iso bulk irq
  * Jeszcze cala warstwa lacza, i te bmRequesty - patrz struktura usb_setup_packet_t
  * A jeszcze przerwania przeciez... no tak przerwania...
  * Adresowanie, resetowanie
  * 
  * 
  * 
  * 
  * 
  * Jak zrozumisz to zrozumisz i uruchomisz USB na kazdym procku
  * Opcja krotsza wziecie libki jak jest - wady? 
  * Zostaje dlug technologiczny - w komercyjnym projekcie i tak i tak odbije sie czkawka na 99% 
  * zalety?
  * Zaoszczedzone sporo czasu nerwow, i zlosci. USB, moim zdaniem jest zaprojektowane bardzo nie zrozumiale
  * 
  * 
  * 
  * I choc ma zalety USB no ma, no ale dlaczego tak to po dziwaczyli,
  * ja rozumiem ze nie jedna strona moze naraz nadawac
  * no nie wiem, dziwnie to porobili. Jeszcze te stany ponazywali sobie J i K 
  * no qrwa
  * pakiety  IN, OUT, sprawdzanie parzystoci - pakiety DATA0, DATA1.
  * potwierdzania - ACK i NAK , do tego jeszcze "STALL"
  * Najgorsze te dane sterujace ughh
  * Bo potem korzystanie i pisanie do kolejek wydaje sie byc znosne
  * jak juz jestesmy po fazie konfiguracji
  * Moje zdanie jest takie, ze jak pewnie sie to pojmie to jakos idzie,
  * Ale no swieza notatka i prawdziwa po probach zrozumienia co maszynisci...
  * mieli na mysli no dramat :(
  * 
  * 
  * Wypunktujmy obecnie nie znane odpowiedzi:
  * 1. W jaki sposob HOST wykrywa podlaczenie urzadzenia? 
  *               - no dobra tymi rezystorami ale jak podepne go gniazda 20 urzadzen o co tu chodzi hmm..
  * 2. Szczegoly dotyczace zasilania urzadzen z szyny usb
  * 3. no nwm, duzo nie jasnosci jest dalej :( - a kod staralem sie zrobic banalny - no sam go od 0 nie napisalem troska kopiowalem.. 
  */

/*
* Ponarzekalem
* Co robi wIndex??? --Numer interfejsu zdefiniowany w deksryptorach kuwa
*
*
* Calkowita liczba endpointow ma byc zdefiniowana w pierwszym desk. konfiguracji? - Nie :) 
* WireShark na windowsie jest ujowy? 
* Jak nie ma odpowiedzi to mi nie pokazuje, mi ze windowsowy host pytal? Nie no niby pokazuje
* 
*
* Dalej cza ponarzekac, cza by narysowac maszynke stanowa i ujac to wszystko, z danymi sterujacymi. 
* Bo czytajac kod musi tego nie ogarne :(
*
*/

/*
* bmRequestType:
Pole bmRequestType zawiera ogólną charakterystyka żądania:
– bit 7 – bit kierunku, źródło pochodzenia danych:
• 0 – kontroler chce wysłać dane do urządzenia lub nie ma żadnych dodatkowych danych do przesłania;
• 1 – kontroler chce odczytać dane z urządzenia;
– bity 6…5 – typ:
• 0 – żądanie standardowe;
• 1 – żądanie specyficzne dla klasy urządzenia;
• 2 – żądanie specyficzne dla dostawcy (ang. vendor);
• 3 – wartość zarezerwowana;
– bity 4…0 – odbiorca:
• 0 – urządzenie;
• 1 – interfejs;
• 2 – punkt końcowy;
• 3 – inny;
• 4…31 – wartości zarezerwowane

Jeśli żądanie kierowane jest do interfejsu, to pole wIndex zawiera numer tego interfejsu. 
Żądanie kierowane do punktu końcowego ma w tym polu adres punktu
końcowego. Pole bRequest specyfikuje dokładnie typ żądania.

Pole wLength określa ilość danych, które mają być przesłane w ramach tego żądania. Gdy realizacja
żądania nie wymaga przesłania danych, to pole wLength ma wartość 0. Kontroler
powinien ustawić wtedy bit kierunku pola bmRequestType na wartość 0. Gdy urządzenie odbierze 
żądanie, w którym pole wLength ma wartość 0, może zignorować
bit kierunku. Pozostałe pola zawierają parametry, których interpretacja zależy od
typu żądania.
*/