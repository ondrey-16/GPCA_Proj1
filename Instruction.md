# Andrzej Wrzesiński - Projekt Shoal Fish Animation

## Program działa w systemie Linux. Wymagane biblioteki wykorzystywane do tworzenia okna graficznego są zawarte w katalogu external, do którego odwołuje się projekt przy budowaniu programu.

### Po zbudowaniu programu można go uruchomić z poziomu terminala za pomocą komendy
```shell
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia ./Shoal_Fish_Animation [ifGPU] [FishCount]
```
### Pierwsze dwie flagi __NV_PRIME_RENDER_OFFLOAD oraz __GLX_VENDOR_LIBRARY_NAME służą do tego, aby wymusić korzystanie z karty graficznej NVIDIA przy renderowaniu okna. Bez tych flag możliwy byłby konflikt w przypadku korzystania z laptopa z kilkoma kartami graficznymi.
### Konieczne jest za wywołaniem programu wpisanie dwóch argumentów: 
- [ifGPU] - 0 lub 1 w zależności od tego, czy uruchamiany ma być program w wersji CPU bądź GPU,
- [FishCount] - dodatnia (<= 30000) liczba ryb tworzonych do obliczeń.

### Okno graficzne zawiera 6 sliderów wykorzystywanych do zmiany parametrów opisujących zachowania ławic ryb (Separation, ALignment, Cohesion). Występuje również przycisk Start/Stop Animation służący do zatrzymywania bądź wznawiania animacji.
### Poniżej sliderów i przycisku znajduje się canvas, na którym rysowana jest animacja ruchu ryb. Ryby podzielone są na dwie ławice - czerwoną i zieloną, o identycznych parametrach fizycznych, ale o niezależnych zachowaniach, która można modyfikować. Do tego ryby dokonują interakcji jedynie z przedstawicielami tego samego koloru, tworząc dwie osobne ławice.
### Można ingerować w ruch ryb, ruszając kursorem po canvas'ie - traktowany jest on wtedy przez wszystkie ryby jako przeszkoda, którą trzeba ominąć.
### W terminalu wypisywane są czasy wykonywania najważniejszych operacji w algorytmach, tj. inicjalizacja danych, aktualizacja czy usuwanie. Dla wersji GPU czasy aktualizacji są również podzielone na podprocesy, dla których osobno liczony jest czas działania.