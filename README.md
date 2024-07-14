# HeterogeneousComputing02

## Installation Python

**Erstellen und aktivieren einer Virtuellen Umgebung**

- Unix
```bash
python -m venv venv
source venv/bin/activate
```

- Windows
```terminal
python -m venv venv
.\venv\Scripts\activate.bat
```


**Installieren der Dependencies**
```bash
pip install -r requirements.txt
```

### Aufgabe 1
```bash
python aufgabe01.py "../generated/600.0/am_modulation.wav" 1024 256 10 
```

``` bash

0.0Hz 11.768118273843129
172.0Hz 11.983011554946168
344.0Hz 12.516820651465373
516.0Hz 13.323224917121772
688.0Hz 14.409347002024731
860.0Hz 15.83392267209015
1032.0Hz 17.768612692709368
1204.0Hz 20.971088998403175
1376.0Hz 36.05897045623092
1548.0Hz 26.50260704525619
1720.0Hz 41.47641425415625
1892.0Hz 30.571313976974828
2064.0Hz 31.815479911581797
2236.0Hz 32.94955975352708
2408.0Hz 23.39775540916137
2580.0Hz 19.332859932086194
2752.0Hz 16.771795801131194
2924.0Hz 14.918096436535645
3096.0Hz 13.460539173040418
3268.0Hz 12.251079174954747
3440.0Hz 11.212726487462819
3612.0Hz 10.300986967564052
Execution time: 1.0787029266357422 seconds
```

### Aufgabe 2
```bash
python aufgabe02.py
```

### Aufgabe 3
```bash
python aufgabe03.py "../generated/600.0/am_modulation.wav" 1024 256 10
```

``` bash
0.0Hz 11.767122196858036
172.0Hz 11.981908950664621
344.0Hz 12.515660227992328
516.0Hz 13.322048190572238
688.0Hz 14.408167849577563
860.0Hz 15.832739605061246
1032.0Hz 17.767409843574264
1204.0Hz 20.969805951910377
1376.0Hz 36.05807613983755
1548.0Hz 26.501778617060737
1720.0Hz 41.475436049785316
1892.0Hz 30.570139751134846
2064.0Hz 31.814340354759246
2236.0Hz 32.94860413460106
2408.0Hz 23.396819729060173
2580.0Hz 19.331888299250465
2752.0Hz 16.770780404069736
2924.0Hz 14.91709131015256
3096.0Hz 13.459564207466215
3268.0Hz 12.250135013945524
3440.0Hz 11.211806468523978
3612.0Hz 10.300086747237918
Execution time: 0.6945679187774658 seconds

```


### Aufgabe 4
```bash
python aufgabe1.py
```


## Installation C

Dependencies:
- cmake
- make
- gcc
- vcpkg


``` bash
vcpkg install
mkdir build
cmake ..
make
```


### Aufgabe 1

**FFTW**

```bash
./aufgabe01 ../../generated/600.0/am_modulation.wav  1024 512 10 
```

``` bash

0Hz 11.768115
172Hz 11.982998
344Hz 12.516828
516Hz 13.323254
688Hz 14.409395
860Hz 15.833987
1032Hz 17.768688
1204Hz 20.971167
1376Hz 36.059133
1548Hz 26.502765
1720Hz 41.476580
1892Hz 30.571478
2064Hz 31.815625
2236Hz 32.949745
2408Hz 23.397979
2580Hz 19.333112
2752Hz 16.772066
2924Hz 14.918375
3096Hz 13.460824
3268Hz 12.251364
3440Hz 11.213011
3612Hz 10.301271

Execution time: 0.368317 seconds
```


**KISS**
```bash
./aufgabe01_kiss ../../generated/600.0/am_modulation.wav  1024 512 10
```

``` bash
0Hz 11.768115
172Hz 11.982998
344Hz 12.516829
516Hz 13.323253
688Hz 14.409395
860Hz 15.833987
1032Hz 17.768688
1204Hz 20.971167
1376Hz 36.059133
1548Hz 26.502765
1720Hz 41.476580
1892Hz 30.571478
2064Hz 31.815624
2236Hz 32.949745
2408Hz 23.397978
2580Hz 19.333112
2752Hz 16.772066
2924Hz 14.918375
3096Hz 13.460823
3268Hz 12.251364
3440Hz 11.213011
3612Hz 10.301271

Execution time: 0.338401 seconds

```


### Aufgabe 2
```bash
./aufgabe02
```

### Aufgabe 3

**FFTW3**

```bash
./aufgabe03 ../../generated/600.0/am_modulation.wav  1024 512 10
```

``` bash
Using 4 cores
0Hz 11.767691
172Hz 11.982387
344Hz 12.516095
516Hz 13.322452
688Hz 14.408543
860Hz 15.833086
1032Hz 17.767724
1204Hz 20.970083
1376Hz 36.058279
1548Hz 26.502233
1720Hz 41.475653
1892Hz 30.570113
2064Hz 31.814281
2236Hz 32.948921
2408Hz 23.397295
2580Hz 19.332401
2752Hz 16.771266
2924Hz 14.917577
3096Hz 13.460132
3268Hz 12.250757
3440Hz 11.212448
3612Hz 10.300730

Execution time: 0.577539 seconds
```

**KISSFFT**

```bash
./aufgabe03_kiss ../../generated/600.0/am_modulation.wav  1024 512 10 
```

``` bash
Using 4 cores
0Hz 11.766965
172Hz 11.981874
344Hz 12.515733
516Hz 13.322175
688Hz 14.408327
860Hz 15.832920
1032Hz 17.767608
1204Hz 20.970071
1376Hz 36.058301
1548Hz 26.502188
1720Hz 41.475645
1892Hz 30.570141
2064Hz 31.814304
2236Hz 32.948900
2408Hz 23.397234
2580Hz 19.332338
2752Hz 16.771247
2924Hz 14.917564
3096Hz 13.460107
3268Hz 12.250724
3440Hz 11.212418
3612Hz 10.300706

Execution time: 0.269912 seconds

```



### Aufgabe 4 (FAULTY)
```bash
./aufgabe04 ../../generated/600.0/am_modulation.wav  1024 512 20
```


``` bash
0Hz 20.416316
5160Hz 20.961247
5332Hz 23.323307
5504Hz 26.932976
5676Hz 23.015559
5848Hz 21.632200
6020Hz 20.639779
13588Hz 20.466026
15136Hz 20.815597
15824Hz 22.441692
15996Hz 22.453258
16168Hz 23.675258
16340Hz 25.751624
16512Hz 20.846123
24596Hz 21.357940
27004Hz 20.087666
27176Hz 21.360276
27348Hz 24.173729
27520Hz 23.122775
38356Hz 21.206502
38528Hz 21.136598

Execution time: 5.962039 seconds
```

## Ergebnisse

**Python**
### Single Process
python .\aufgabe01.py ..\generated\600.0\am_modulation.wav 512 {SHIFT} 10
{SHIFT = 1}     Execution time: 230.90340924263 seconds
{SHIFT = 2}     Execution time: 114.44869232177734 seconds
{SHIFT = 4}     Execution time: 56.12582802772522 seconds
{SHIFT = 8}     Execution time: 28.75834631919861 seconds

## Multiprocessing (4 Cores)
python .\aufgabe03.py ..\generated\600.0\am_modulation.wav 512 {SHIFT} 10
{SHIFT = 1}     Execution time: 101.88142442703247 seconds
{SHIFT = 2}     Execution time: 46.36954140663147 seconds
{SHIFT = 4}     Execution time: 18.755671977996826 seconds
{SHIFT = 8}     Execution time: 9.485113382339478 seconds


## OpenCL
python .\aufgabe04.py ..\generated\600.0\am_modulation.wav 512 {SHIFT} 10
{SHIFT = 1}     Execution time: 4481.816161155700684 seconds
{SHIFT = 2}     Execution time: 2189.94771848737698 seconds
{SHIFT = 4}     Execution time: 1429.084727748985432 seconds
{SHIFT = 8}     Execution time: 615.4815285205841 seconds


**C**
### Single Process
.\aufgabe01 ..\..\generated\600.0\am_modulation.wav 512 {SHIFT} 10

{SHIFT = 1}     Execution time: 37.135065 seconds
{SHIFT = 2}     Execution time: 18.613663 seconds
{SHIFT = 4}     Execution time: 9.609901 seconds
{SHIFT = 8}     Execution time: 5.020272 seconds


.\aufgabe01_kiss ..\..\generated\600.0\am_modulation.wav 512 {SHIFT} 10

{SHIFT = 1}     Execution time: 112.489262 seconds
{SHIFT = 2}     Execution time: 55.761896 seconds
{SHIFT = 4}     Execution time: 27.865394 seconds
{SHIFT = 8}     Execution time: 13.961786 seconds


## Multiprocessing (4 Cores)
aufgabe03 ..\..\generated\600.0\am_modulation.wav 512 {SHIFT} 10
{SHIFT = 1}     Execution time: 179.262158 seconds
{SHIFT = 2}     Execution time: 89.501099 seconds
{SHIFT = 4}     Execution time: 44.851196 seconds
{SHIFT = 8}     Execution time: 22.519973 seconds

aufgabe03_kiss ..\..\generated\600.0\am_modulation.wav 512 {SHIFT} 10
{SHIFT = 1}     Execution time: 59.587278 seconds
{SHIFT = 2}     Execution time: 29.900206 seconds
{SHIFT = 4}     Execution time: 15.092772 seconds
{SHIFT = 8}     Execution time: 7.534932 seconds


## OpenCL
python .\aufgabe04.py ..\generated\600.0\am_modulation.wav 512 {SHIFT} 10
{SHIFT = 1}     Execution time: 3974.4527066506876 seconds
{SHIFT = 2}     Execution time: 1931.852423823516  seconds
{SHIFT = 4}     Execution time: 1199.466097256576 seconds
{SHIFT = 8}     Execution time: 549.4815285205841 seconds
