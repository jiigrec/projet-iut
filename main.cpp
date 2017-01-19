#include <iostream>
#include <istream>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <dirent.h> 
#include <stdio.h> 
#include <random>
#include <fstream>
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>

using namespace std;


//Index des options de configuration
const vector <string> VControls {"J1Up", "J1Down", "J1Left", "J2Right", "J2Up", "J2Down", "J2Left", "J2Right", "pause"};
const vector <string> VColors {"ColorPlayer1", "ColorPlayer2","ColorFood"};
const string languageSetting = "langue";
const string foodSetting = "food";

//Constantes de l'affichage du personnage et des blancs
const char KPlayerUp ('^');
const char KPlayerDown ('v');
const char KPlayerLeft ('<');
const char KPlayerRight ('>');
const char KEmpty (' ');

//classe myparams
struct CMyParam {
    map <string, char> Controls;
    char food;
    map <string, string> Colors;
    map <string, string> GameStrings;
    bool IA;
    unsigned difficulty;
    unsigned randomFrequency;
    unsigned endTime;
};


//Efface l'écran
void ClearScreen ()
{
    cout << "\033[H\033[2J";
}


//Liste des couleurs disponibles
const string KReset   ("0");
const string KNoir    ("30");
const string KRouge   ("31");
const string KVert    ("32");
const string KJaune   ("33");
const string KBleu    ("34");
const string KMAgenta ("35");
const string KCyan    ("36");


//Change la couleur des prochains caractères affichés
void Couleur (const string & coul)
{
    cout << "\033[" << coul <<"m";
}


//Types de la grille de jeu
typedef vector <char> CVLine; // un type représentant une ligne de la grille
typedef vector <CVLine> CMatrix; // un type représentant la grille
typedef pair   <unsigned, unsigned> CPosition; // un type représentant une coordonnée dans la grille

typedef pair <unsigned, unsigned> CTerminalSize; // Un type qui représente la taille de la fenetre

//Stockage des paramètres du Terminal, pour changer le mode entre canonique et non canonique
struct termios saved_attributes;

/*Variables de configuration, initialisées avec leurs valeurs par défaut
bool IA = false;
unsigned difficulty = 1;
int randomFrequency = 3; */


//Variable contenant les paramètres
CMyParam Settings;


//Variables utilisées pendant le jeu
CMatrix Mat;
CPosition Player1;
CPosition Player2;
unsigned ScoreJ1;
unsigned ScoreJ2;

//Variables qui gardent le temps
unsigned long Time;
unsigned randomCounter = 0;



//Donne la liste des fichiers de langue présents
vector<string> getLanguages() {
   vector<string> Langues;
   /* Source : http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c */
   DIR  *d;
  struct dirent *dir;
  d = opendir("./Nos_fichiers/langue");
  if (d)
  {
    string File;
    while ((dir = readdir(d)) != NULL)
    {
      File = dir->d_name;
      if (File.find(".yaml") < File.size())
          Langues.push_back(File);
    }

    closedir(d);
  }
    return Langues;
}


//Définit les chaines localisées en fonction d'un fichier de langue
void setLanguage(string & LanguageFile) {
    ifstream stream ("Nos_fichiers/langue/" + LanguageFile);
    string str;
    string key;
    while (!stream.eof()) {
        stream >> key;
        stream >> str;
        stream >> str;
        replace(str.begin(), str.end(), '_', ' ');
        Settings.GameStrings[key] = str;
    }
    stream.close();
}

//Définit la langue à partir d'une locale simplifiée, comme FR pour francais
void setLanguageFromLocale(string & Locale) {
      vector<string> Languages = getLanguages();
      for (unsigned i = 0; i < Languages.size(); ++i) {
          if (Languages[i].find(Locale) < Languages[i].size()) {
              setLanguage(Languages[i]);
              return;
          }
      }
}

//Charge les paramètres du fichier config.yaml
void LoadParams(CMyParam & MyParams) {
    ifstream stream("Nos_fichiers/config.yaml");
    string str;
    char c;
    string key;
    while (!stream.eof()) {
        stream >> str;
        if (str == "#") {
            stream >> str;
            while (str != "#")
                stream >> str;
            stream >> str;
        }
        if (str == foodSetting) {
            stream >> c;
            stream >> c;
            MyParams.food = c;
        }
        if (str == languageSetting) {
            stream >> str;
            stream >> str;
            setLanguageFromLocale(str);
        }
        if (find(VColors.begin(), VColors.end(), str) != VColors.end()) {
            key = str;
            stream >> str;
            stream >> str;
            MyParams.Colors[key] = str;
        }
        if (find(VControls.begin(), VControls.end(), str) != VControls.end()) {
            key = str;
            stream >> c;
            stream >> c;
            MyParams.Controls[key] = c;
        }
    }
    stream.close();
}

//Défénit en mémoire les valeurs par défaut de configuration
void setDefaults(CMyParam & MyParams) {
   MyParams.difficulty = 1;
   MyParams.IA = false;
   MyParams.randomFrequency = 3;
   MyParams.endTime = 60;
}


//Donne le code couleur à partir d'une couleur en language commun
string getColor(string & color) {
    if (color ==  "noir")
        return KNoir;
    if (color == "rouge")
        return KRouge;
    if (color == "vert")
        return KVert;
    if (color == "jaune")
        return KJaune;
    if (color == "bleu")
        return KBleu;
    if (color == "magenta")
        return KMAgenta;
    if (color == "cyan")
        return KCyan;
    return KReset;
}

 void MenuTuto()
    {
      //  AffichFich("MODEDEJEU.txt");
        char Choix;
        char quit;
        cin>>Choix;
        ClearScreen();
        switch(Choix)
        {
        case'1':
         //   ShowFile("TUTORIEL.txt");
            cin>>quit;
            while(quit!='5')
                cin>>quit;
            ClearScreen();
            break;
        default:
            break;
        }
 }


 //Règle la difficulté du jeu
 void setDifficulty() {
     int choix = 0;
     while (choix < 1 || choix > 3) {
         cout << "Choisir la difficulté: " << endl;
         cout << "1. " << "Facile" << endl;
         cout << "2. " << "Moyen" << endl;
         cout << "3. " << "Difficile" << endl;
         cout << "Entrez votre choix " << ":" << endl;
         cin >> choix;
     }
     Settings.difficulty = choix;
     Settings.randomFrequency = 4 - choix;
 }

 void languageMenu() {

 }

//Affiche le fichier d'aide
 void showHelp() {
     ClearScreen();
     ifstream stream ("Nos_fichiers/langue/" + Settings.GameStrings["HelpFile"]);
     string str;
     while (!stream.eof()) {
         getline(stream, str);
         cout << str << endl;
     }
     char c = 'y';
     cout << "Appuyez sur 1 puis entrée pour revenir." << endl;
     while (c != '1') {
         cin >> c;
     }

 }

//Enregistre les options choisies
 bool setOption(int & Choix) {
     switch (Choix) {
     case 1:
         Settings.IA = !Settings.IA;
         return false;
         break;
     case 2:
         setDifficulty();
         return false;
         break;
     case 3:
         languageMenu();
         return false;
         break;
     case 4:
         showHelp();
         return false;
         break;
     case 5:
         return true;
     default:
         return true;
         break;
     }
 }


 //Affiche le menu principal
bool mainMenu() {
    bool ready = false;
    while (!ready) {
        cout << "Menu : " << endl << "1. ";
        if (Settings.IA)
            cout << "Mode 1 joueur" << ".";
        else
            cout << "Mode 2 joueurs" << ".";
        cout << " Choisissez pour changer" << "." << endl;
        cout << "2. " << "Changer difficulté"<<". "<<"Actuellement "<<": " << Settings.difficulty << endl;
        cout << "3. " << "Changer la langue" << endl;
        cout << "4. " << "Aide" << endl;
        cout << "5. " << "Jouer" << endl;
        cout << "6. " << "Quitter" << endl;
        cout << "Entrez votre choix :" << endl;
        int choix;
        cin >> choix;
        while (true)
            if (choix == 5)
                return false;
            if (choix <= 0 || choix > 5)
                continue;
        ready = setOption(choix);
    }
    return true;

}




/* Code copié depuis :
http://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
*/

void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode (void)
{
  struct termios tattr;

  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
  {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
  }

  /* Save the terminal attributes so we can restore them later. */
   tcgetattr (STDIN_FILENO, &saved_attributes);
   atexit (reset_input_mode);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 0;
  tattr.c_cc[VTIME] = 3;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

/* Fin du code copié */


//Démarre le chronomètre
void startTimer() {
    Time = time(NULL);
}

//Donne le nombre de secondes écoulées depuis le début du chrono
unsigned getSecondsElapsed() {
    return ((unsigned) time(NULL) - Time);
}

//Donne le temps restant des joueurs
int getTimeLeft() {
    return Settings.endTime - getSecondsElapsed();
}


//Mets le jeu en pause
void gamePause() {
    ClearScreen();
    unsigned elapsedTime = getSecondsElapsed();
    reset_input_mode();
    char c = 'y';
    cout << "Jeu en pause. Appuyez sur 1 puis entrée pour reprendre." << endl;
    while (c != '1') {
        cin >> c;
    }
    set_input_mode();
    Time = time(NULL) - elapsedTime;
    ClearScreen();
}

//Fait apparaitre un trésor sur la carte
void popFood(CMatrix & Mat) {
    if (randomCounter == Settings.randomFrequency) {
        randomCounter = 0;
        random_device rd;
        mt19937 mt(rd());
        uniform_int_distribution<int> vert(1, Mat.size());
        uniform_int_distribution<int> hor(1, Mat[0].size());
        CPosition food = pair<int, int> (vert(mt), hor(mt));
        while (food == Player1 || food == Player2) {
            food = pair<int, int> (vert(mt), hor(mt));
        }
        Mat[food.first][food.second] = Settings.food;
    } else
        ++randomCounter;
}

//Affiche l'interface du jeu ainsi que la grille
void  ShowMatrix (const CMatrix & Mat) {
    ClearScreen();
    Couleur(KReset);
    cout << "Temps restant " << ": " << getTimeLeft() << " Score J1" << ": " << ScoreJ1 << " Score J2" << ": " << ScoreJ2 << endl;
    for (unsigned i = 0; i <= Mat[0].size() + 1; ++i) cout << "=";
    cout << endl;
    for (unsigned i = 0; i < Mat.size(); ++i) {
        cout << "|";
        for (unsigned j = 0; j < Mat[i].size(); ++j) {
            if (Mat[i][j] == KEmpty) {
                cout << KEmpty;
            }
            if (i == Player1.first && j == Player1.second) {
                Couleur(KVert);
                cout << Mat[i][j];
                Couleur(KReset);
            }
            if (i == Player2.first && j == Player2.second) {
                Couleur(KRouge);
                cout << Mat[i][j];
                Couleur(KReset);
            }
            if (Mat[i][j] == Settings.food) {
                Couleur(KBleu);
                cout << Settings.food;
                Couleur(KReset);
            }
        }
        cout << "|" << endl;

    }
    for (unsigned i = 0; i <= Mat[0].size() + 1; ++i) cout << "=";
    cout << endl;
}


//Initialise la grille de jeu
void InitMat (CMatrix & Mat, unsigned NbLine, unsigned NbColumn, CPosition & PosPlayer1, CPosition & PosPlayer2) {
    Mat.resize(NbLine);
    CVLine Line;
    Line.resize(NbColumn);
    for (unsigned i = 0; i < Line.size(); ++i) {
        Line[i] = KEmpty;
    }
    for (unsigned i = 0; i < Mat.size(); ++i) {
        Mat[i] = Line;
    }
    PosPlayer1 = {0,0};
    Mat[PosPlayer1.first][PosPlayer1.second] = KPlayerDown;
    PosPlayer2 = {NbLine - 1, NbColumn - 1};
    Mat[PosPlayer2.first][PosPlayer2.second] = KPlayerUp;
}


//Retourne si le joueur est le joueur 1 ou non, en fonction de sa position
bool isFirstPlayer(CPosition & Pos) {
    return (Pos == Player1); //Expression booléenne
}


//Fonction qui gère les collisions
bool checkEat(char & Object, CPosition & PosObject,char & Player, CPosition & PosPlayer) {
    switch (Object) {
    case KPlayerDown:
    case KPlayerUp:
    case KPlayerLeft:
    case KPlayerRight: // C'est un joueur
        //Si les joueurs se font face, ils ne peuvent pas se manger.
        if (PosObject.first > PosPlayer.first && Object == KPlayerUp && Player == KPlayerDown)
            return false;
        if (PosObject.first < PosPlayer.first && Object == KPlayerDown && Player == KPlayerUp)
            return false;
        if (PosObject.second > PosPlayer.second && Object == KPlayerLeft && Player == KPlayerRight)
            return false;
        if (PosObject.second < PosPlayer.second && Object == KPlayerRight && Player == KPlayerLeft)
            return false;
        //Ici, un joueur mange l'autre. Sinon, la fonction a déjà fini son éxécution.
        if (isFirstPlayer(PosPlayer))
            ScoreJ1 = ScoreJ1 + 5;
        else
            ScoreJ2 = ScoreJ2 + 5;
        break;

    default:  // Pas un joueur
        if (Object == Settings.food) {
            if (isFirstPlayer(PosPlayer))
                ++ScoreJ1;
            else
                ++ScoreJ2;
        }
        break;
    }

    return true;
}


//Fonction pour déplacer un joueur
void MoveToken (CMatrix & Mat, char Move, CPosition  & Pos) {
    CPosition OldPos = Pos;
    char Player;
    Mat[Pos.first][Pos.second] = KEmpty;
    if (Move == Settings.Controls["J1Up"] || Move == Settings.Controls["J2Up"]) {
        if (Pos.first > 0) Pos.first = Pos.first - 1;
        Player = KPlayerUp;
    }
    if (Move == Settings.Controls["J1Left"] || Move == Settings.Controls["J2Left"]) {
        if (Pos.second) Pos.second = Pos.second - 1;
        Player = KPlayerLeft;
    }
    if (Move == Settings.Controls["J1Right"] || Move == Settings.Controls["J2Right"]) {
        if (Pos.second < Mat[0].size() - 1) Pos.second = Pos.second + 1;
        Player = KPlayerRight;
    }
    if (Move == Settings.Controls["J1Down"] || Move == Settings.Controls["J2Down"]) {
        if (Pos.first < Mat.size() - 1) Pos.first = Pos.first + 1;
        Player = KPlayerDown;
    }

    if (Mat[Pos.first][Pos.second] != KEmpty)
        if (!checkEat(Mat[Pos.first][Pos.second], Pos, Mat[OldPos.first][OldPos.second], OldPos))
            Pos = OldPos;
    Mat[Pos.first][Pos.second] = Player;
}

//Fonction pour lire les touches pressées
void readInput(char & Input) {
    if (Input == Settings.Controls["J1Up"] || Input == Settings.Controls["J1Down"] || Input == Settings.Controls["J1Left"] || Input == Settings.Controls["J1Right"])
        MoveToken(Mat, Input, Player1);
    if (Input == Settings.Controls["J2Up"] || Input == Settings.Controls["J2Down"] || Input == Settings.Controls["J2Left"] || Input == Settings.Controls["J2Right"])
        MoveToken(Mat, Input, Player2);
    if (Input == Settings.Controls["pause"])
        gamePause();
}


//Fonction qui donne la taille de la fenêtre
void getWindowSize(CTerminalSize & Size) {
    struct winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    Size.first = size.ws_row;
    Size.second = size.ws_col;
}

// Affiche le menu des résultats
bool win(unsigned Player, unsigned & Score1, unsigned & Score2) {
        cout << "Félicitations au Joueur " << Player << " ! Il gagne avec " << Score1 << " points contre " << Score2 << "points." << endl;
        while (true) {
            cout << "1. " << "Revenir au menu" << endl;
            cout << "2. " << "Quitter" << endl;
            int choix;
            cin >> choix;
            if (choix == 1)
                return true;
            if (choix == 2)
                return false;
        }
}


// Boucle d'une partie jouée
bool ppal () {
      bool Playing = true;
      CTerminalSize WindowSize;
      getWindowSize(WindowSize);
      InitMat(Mat, WindowSize.first - 6,WindowSize.second - 2,Player1,Player2);
      ShowMatrix(Mat);
      char Saisie;
      set_input_mode();
      ScoreJ1 = 0;
      ScoreJ2 = 0;
      while(Playing) {
            ShowMatrix(Mat);
            read (STDIN_FILENO, &Saisie, 1); //Lit la touche pressée.
            readInput(Saisie);
            popFood(Mat);
            if (!(getTimeLeft() >= 0))
                break;
      }
      reset_input_mode();
      if (ScoreJ1 > ScoreJ2)
         return win(1, ScoreJ1, ScoreJ2);
      else
         return win(2, ScoreJ2, ScoreJ1);
}





int main()
{
    LoadParams(Settings);
    setDefaults(Settings);
    bool playing = true;
    while(playing) {
        playing = mainMenu();
        playing = ppal();
    }


    return 0;
}



