# FFSlib
Ferrara FileSystem Lib: Proggetto Sistemi Operativi di Pertile e Maffei

# 1 Introduzione
Scopo di questo progetto è di creare una libreria user-level che implementa un file-system. La libreria
permette alle applicazioni di accedere a files e directories memorizzati su un disco virtuale. Il disco è
realizzato mediante un’altra libreria user-level dummydisk che vi verrà fornita.

# 2 FFSlib Specification
La libreria FFSlib vede il disco come un array di blocchi ciascuno di dimensione uguale o maggiore del
settore del disco. Il file-system organizza il disco in diverse aree nel seguente modo:
  
  • Superblock area: un blocco del file-system contenente un identificatore intero corrispondente al
  tipo di file-system; si assuma che il tipo del nostro file-system sia identificato dal codice `0x1717`;
  
  • 1 blocco per memorizzare la bit-map di stato (libero,occupato) degli i-node;
  
• 1 blocco per memorizzare la bit-map di stato (libero,occupato) degli blocchi dato;

• I-node-list area: un insieme di blocchi ciascuno contenente un i-node-block;

• Data blocks area: l’insieme di blocchi dato del file-system in cui sono memorizzati i dati di file e
directory.

La dimensione del blocco ovviamente limita la dimensione delle bit-map, che di conseguenza limitano la
dimensione del file-system. Dimensioni tipiche per un blocco sono 1,2,4,8 Kbytes.

    OPZIONALE: organizzare il disco in gruppi ciascuno organizzato come sopra; mettere tanti gruppi
    fino ad occupare l’intero disco.

Ogni i-node-block deve contenere almeno le seguenti informazioni:

• un valore intero che specifica il tipo, file o directory;

• un intero che specifica la dimensione in byte;

• una lista di indirizzi di blocchi dato; si assuma di utilizzare solo indirizzamento diretto e che un
file può occumare al massimo `MAX BLOCK FILE` (e.g 32 o 64) blocchi dato.

Per quanto riguarda i nomi, si assuma che il nome dei file o directory sia lungo al massimo `MAX_FILENAME_LEN`
(e.g. 16 caratteri), e ogni pathname sia lungo al massimo `MAX_PATHNAME_LEN` caratteri (e.g. 256 caratteri).
Si può assumere inoltre che tutti i pathname siano assoluti ovvero che i pathname si riferiscono tutti
alla root del file-system, es.: /usr/bin/myprog.
Quindi, l’accesso al file myprog implica innanzitutto l’accesso alla directory root allocata per all’i-node
zero; conseguentemente bisogna verificare se esiste la directory usr, determinare il suo i-node e ripetere
l’operazione finch`e non si ha accesso all’i-node associato al file prog da cui possiamo determinare il
blocco dati da scrivere o leggere.

Per rendere più efficienti accessi consecutivi allo stesso file, dopo il primo accesso le informazioni relative
al file devono essere memorizzate nella tabella `Open File Table`. Questa tabella ha una entry per ogni
file in cui sono riportate tutte le informazioni per l’accesso al file. Quando un file viene aperto per la
prima volta, le sue informazioni devono essere memorizzate nella prima riga i libera della tabella; il
valore i detto `file-descriptor` è il valore ritornato dalla funzione di open e utilizzato come parametro dalle
funzioni di `read`, `write`, `close` e `seek`. La entry nella tabella dei file aperti deve essere mantenuto sino alla
chiusura del file.

    OPZIONALE: Per supportare anche i path relativi bisogna tenere traccia della current working directory.

Le operazioni di read e write fanno riferimento al `current file pointer`. Dopo l’apertuna del file il current
file pointer punta all’inizio del file. Una operazione di read di N bytes incrementa il file-pointer esattamente
di N bytes. Una successiva operazioni di read di M bytes ritorna M bytes successivi a primi N,
e incrementa il puntatore di M bytes. L’operazioni di seek permette di posizionare il file pointer in una
specifica posizione del file.

# 2.1 File System API

• `int FS Init()`: questa funzione deve essere chiamata una sola volta per creare il disco (vedi API
libreria disklib) e le strutture dati necessarie per manipolare files e directories

# 2.2 File Access API
La libreria FSlib comprende tre tipi di API:

• API generiche:

– `int FFS_Init(char * diskname)`: questa funzione prende come argomento il nome di un
disco; se il disco esiste la funzione invoca la funzione `Disk_Load(diskname)` e controlla se il
disco contiene un filesystem di tipo `0x1717`. In caso positivo ritorna con successo, altrimenti
ritorna il valore -1 e setta la variabile `osErrno = E_GENERAL`.
Se il disco non esiste, allora invoca la funzione `Disk_Create(diskname)` per creare un nuovo
disco. Quindi inizializza il disco con un nuovo file-system di tipo `0x1717`, e crea la root
directory.

– `int FFS_Sync(char * diskname)`: questa funzione quando invocata, chiude tutti i file aperti,
e successivamente invoca la funzione `Disk_Save(diskname)`.

• API per la gestione dei files

– `int File_Create(char * name)`: questa funzione crea un nuovo file di nome *name, dimensione
zero e non aperto. Se il file esiste ritorna -1 e setta la variabile `osErrno = E_CREATE`;

– `int File_Open(char * name)`: questa funzione apre il file di nome *name e ritorna un valore
intero maggiore o uguale a zero detto `file-descriptor`, utilizzato per accedere al file mediante
le funzioni di `read`, `write`, `close` e `seek`. Se il file non esiste, ritorna -1 e setta il valore della
variabile `osErrno = E_NO_SUCH_FILE`. Se è stato raggiunto il numero massimo di file aperti,
ritorna -1 e setta `osErrno = E_TOO_MANY_OPEN_FILES`;

– `int File_Close(int fd)`: questa funzione chiude il file avente file-descriptor fd. Se il file
non è aperto ritorna -1 e setta `osErrno = E_BAD_FD`;

– `int File_Read(int fd, void * buffer, int size)`: questa funzione legge al più size
bytes a partire dal file-pointer del file avente file-descriptor fd e li copia nel buffer buffer. Se
il fine non è aperto ritorna -1 e setta `osErrno = E_BAD_FD`. Il numero di byte letti può essere
inferiore a size – anche zero – se il file contiene un numero di bytes inferiore e il pointer ha
ranggiunto la fine del file.

– int `File_Write(int fd, void * buffer, int size)`: questa funzione scrive al più size
bytes a partire dal file-pointer del file avente file-descriptor fd e li copia nel buffer buffer. Se
il fine non è aperto ritorna -1 e setta `osErrno = E _BAD _FD`. Il numero di byte scritti può essere
inferiore a size – anche zero – se il file contiene un numero di bytes inferiore e il pointer ha
ranggiunto la fine del file.

– `int File_Seek(int fd, int offset)`: questa funzione permette di aggiornare il puntatore
al file. Il valore di offset può essere negativo o positivo e la nuova posizione è sommando
offset alla posizione corrente. Se il valore della nuova posizione è negativo o maggiore della
dimensione del file, allora l’operazione di seek fallisce, il puntatore al file non viene aggiornato,
e la variabile `osErrno = E_SEEK_OUT_OF_BOUNDS`. Se il file non è aperto la l’operazione
fallisce e la variabile `osErrno = E_BAD_FD`. In caso di successo ritorna la nuova locazione
del file pointer.

– `int File_Unlink(char * file)`: questa funzione cancella un file. Il file deve essere rimosso
dalla directory che lo contiene, la entry nella tabella dei file aperti deve essere eliminata, e
l’i-node i i blocchi dati da esso occupati liberati. Se il file non esiste, ritorna il valore -1 e
setta la variabile `osErrno = E_NO_SUCH_FILE`. Se il file è aperto, ritorna -1 e `osErrno = E_FILE_IN_USE`
e il file NON viene cancellato.

• API per la gestione delle directories

– `int Dir_Create(char * dirpath)`: questa funzione crea una di directory di nome dirpath.
La nuova directory deve essere aggiunta alla lista degli elementi della directory padre. Ad
esempio: `Dir_Create("/usr/lib")` crea la directory lib se e solo se la directory ”/” e la
directory ”/usr” esistono già. In caso di fallimento ritorna -1 e setta la variabile `osErrno = E_CREATE`.

– `int Dir_Read(char * dirpath, void * buffer)`: questa funzione copia un insieme di entry
(file e directory) della directory dirpath e li copia nel buffer buffer. Ogni entry è grande
20 bytes, e contiene 16 bytes di nome e 4 bytes per codificare l’indice (integer) del corrispondente
i-node. La dimensione del buffer deve essere uguale a 20 bytes per il numero di entry
della directory.

– `int Dir_Unlink(char * dirpath)`: questa funzione permette di cancellare la directory dirname,
liberando il corrispondente i-node e il blocco dato occupato, e rimuovendo la sua entry dalla
directory padre. La funzione fallisce se la directory che si vuole rimuovere NON è vuota,
ritorna -1 e setta la variabile `osErrno = E_DIR_NOT_EMPTY` e la directory non viene rimossa.
Analogamente, se si tenta di rimuove la root directory la chiamata fallisce e `osErrno = E_ROOT_DIR`.

Tutte le funzioni ritornano il valore 0 in caso di successo; in caso di errore ritornano il valore -1 e la
la variabile globale `osErrno` è inizializzata con un appropriato codice di errore. I codici di errore sono
definiti nel file `fslib.h`.

# 3 Disco Virtuale
Il file system memorizza file directory e strutture dati del file-system su un disco virtuale realizzato dalla
libreria `dummydisk`.
La libreria realizza un disco di `NUM_SECTORS` settori ciascuno di dimensione `SECTOR_SIZE` bytes.
Il disco virtuale può essere creato mediante la funzione `Disk_Create()`. Dopo la creazione il disco
contiene solo zeri, e la sua immagine e salvata in memoria. L’operazione di `Disk_Save()` permette di
salvare l’immagine di memoria dopo aver finito di utilizzare il disco oppure ad ogni operazione di sync
richiesta da file-system. La funzione `Disk_Load()` permette di caricare l’immagine del disco in memoria.
Per motivi diefficienza prima di utilizzare il disco virtuale la sua immagine deve essere caricata in
memoria; questo può essere ottenuto mediante la funzione `Disk_Create` se si è creato un nuovo disco,
oppure mediante la funzione `Disk_Load` se si vuole utilizzare un disco esistenete.
Le operazioni di `Disk_Read()` e `Disk_Write()` permettono rispettivamente di leggere e scrivere un settore
del disco.
La libreria mette a disposizione le seguenti costanti e API che possono essere utilizzate per realizzare la
libreria `FFSlib`:

• `NUM_SECTORS`: numero di settori del diso.

• `SECTOR _SIZE`: dimensione in bytes di ciascun settore.

• `Disk_Create()`: questa funzione crea un disco virtuale di dimensione `NUM_SECTORS × SECTOR_SIZE` bytes. Il disco è mantenuto in memoria finchè non viene effettuata l’operazione di `Disk_Sync()`.

• `Disk_Load(char * diskname)`: questa funzione carica in memoria il disco di nome diskname; se il disco non esiste la variabile `diskErrno` è inizializzata con il valore `E_OPENING_FILE`. Se il file letto no ha la dimensione attesa 
`NUM_SECTORS × SECTOR_SIZE` la variabile `diskErrno` è inizializzata con il valore `E_READING_FILE`. In caso di successo ritorna il valore zero. Questa funzione deve essere chiamata una sola prima di effettuare qualunque operazione sul file system.

• `Disk_Save(char * diskname)`: questa funzione salva l’immagine in memoria del disco sul file diskname. Questa funzione deve essere invocata ogni volta che si vuole effettuare una operazione di sync tra l’immagine in memoria e quella su file del disco.

• `Disk_Write(int sector, char * buffer)`: questa funzione scrive il settore sector del disco con
i dati contenuti nel buffer buffer. Il buffer deve essere della stessa dimensione in byte del settore.

• `Disk_Read(int sector, char * buffer)`: questa funzione legge il settore sector del disco e
memorizza i dati contenuti nel buffer buffer. Il buffer deve essere della stessa dimensione in byte
del settore.

Tutte le operazioni ritornano il valore 0 in caso di successo; in caso di fallimento o errore ritornano il
valore -1, e la variabile globale `diskErrno` è inizializzata con un appropriato codice di errore.
