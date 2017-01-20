/*!
 * \file   main.cpp
 * \authors Jean-Yves Roda, Marc-Antoine Cartier, Antoine De La Fuente, Rémi Brizon
 * \date Vendredi 20 Janvier 2017
 * \brief   Terminal's color management
 *          beginning of the project titled "catch me if you can"
 */



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

//! Liste des options des commandes
const vector <string> VControls {"J1Up", "J1Down", "J1Left", "J1Right", "J2Up", "J2Down", "J2Left", "J2Right", "pause"};
//! Liste des options de couleur
const vector <string> VColors {"ColorPlayer1", "ColorPlayer2","ColorFood"};
//! Option de langue
const string languageSetting = "langue";
//! Option du trésor a "manger"
const string foodSetting = "food";

//! Joueur allant en haut
const char KPlayerUp ('^');
//! Joueur allant en bas
const char KPlayerDown ('v');
//! Joueur allant a gauche
const char KPlayerLeft ('<');
//! Joueur allant à droite
const char KPlayerRight ('>');
//! Espace vide
const char KEmpty (' ');

/*!
 * \struct Structure regroupant les parametres du jeu
 */
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


/*!
 * \brief Efface l'écran du terminal
 */
void ClearScreen ()
{
    cout << "\033[H\033[2J";
}


//! Code couleur nul (reinitialisation)
const string KReset   ("0");
//! Code couleur noir
const string KNoir    ("30");
//! Code couleur rouge
const string KRouge   ("31");
//! Code couleur vert
const string KVert    ("32");
//! Code couleur jaune
const string KJaune   ("33");
//! Code couleur bleu
const string KBleu    ("34");
//! Code couleur magenta
const string KMAgenta ("35");
//! Code couleur cyan
const string KCyan    ("36");

/*!
 * \brief Change la couleur des prochains caractères affichés
 * \param[in] Code de la couleur à afficher
 */
void Couleur (const string & coul)
{
    cout << "\033[" << coul <<"m";
}


/*!
 * \brief Type représentant une ligne dans la grille du jeu
 */
typedef vector <char> CVLine;
/*!
 * \brief Type représentant la grille du jeu
 */
typedef vector <CVLine> CMatrix;
/*!
 * \brief Type représentant la position d'un joueur dans la grille
 */
typedef pair   <unsigned, unsigned> CPosition; // un type représentant une coordonnée dans la grille
/*!
 * \brief Type représentant la taille de la fenetre
 */
typedef pair <unsigned, unsigned> CTerminalSize; // Un type qui représente la taille de la fenetre

//! Configuration du terminal, notemment le mode d'entrée
struct termios saved_attributes;

//! Réglages du jeu
CMyParam Settings;

//! Grille de jeu
CMatrix Mat;
//! Position du joueur 1
CPosition Player1;
//! Position du joueur 2
CPosition Player2;
//! Score du joueur 1
unsigned ScoreJ1;
//! Score du joueur 2
unsigned ScoreJ2;

//! Temps au debut du chronometre
unsigned long Time;
//! Compteur du generateur aleatoire
unsigned randomCounter = 0;


#ifdef __APPLE__
    /*!
     * \brief Fonction pour jouer la musique
     */
    void playMusic() {
        system("afplay ../G1_Brizon_Cartier_DeLaFuente_Roda/Nos_fichiers/music.wav &");
    }
    /*!
     * \brief Fonction pour arreter la musique
     */
    void stopMusic() {
        system("killall afplay");
    }
#else
    /*!
     * \brief Fonction pour jouer la musique
     */
    void playMusic() {
        system("aplay ../G1_Brizon_Cartier_DeLaFuente_Roda/Nos_fichiers/music.wav &");
    }
    /*!
     * \brief Fonction pour arreter la musique
     */
    void stopMusic() {
        system("killall aplay");
    }
#endif


/*!
 * \brief Fonction pour avoir la liste des langues disponibles
 * \return Un vecteur contenant les langues
 */
vector<string> getLanguages() {
   vector<string> Langues;
   /* Source : http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c */
   DIR  *d;
  struct dirent *dir;
  d = opendir("../G1_Brizon_Cartier_DeLaFuente_Roda/Nos_fichiers/langue");
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

/*!
 * \brief Definit la langue du jeu
 * \param Le nom du fichier qui contient les parametres de langue
 */
void setLanguage(string & LanguageFile) {
    ifstream stream ("../G1_Brizon_Cartier_DeLaFuente_Roda/Nos_fichiers/langue/" + LanguageFile);
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


/*!
 * \brief Definit la langue du jeu a partir d'une locale
 * \param La locale de la langue a changer
 */
void setLanguageFromLocale(string & Locale) {
      vector<string> Languages = getLanguages();
      for (unsigned i = 0; i < Languages.size(); ++i) {
          if (Languages[i].find(Locale) < Languages[i].size()) {
              setLanguage(Languages[i]);
              return;
          }
      }
}

/*!
 * \brief Charge les parametres du jeu a partir du fichier config.yml
 * \param Parametres du jeu
 */
void LoadParams(CMyParam & MyParams) {
    ifstream stream("../G1_Brizon_Cartier_DeLaFuente_Roda/Nos_fichiers/config.yaml");
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

/*!
 * \brief Definit quelques parametres par defaut
 * \param Parametres du jeu
 */
void setDefaults(CMyParam & MyParams) {
   MyParams.difficulty = 1;
   MyParams.IA = false;
   MyParams.randomFrequency = 3;
   MyParams.endTime = 60;
}

/*!
 * \brief Donne le code couleur d'une couleur
 * \param La couleur recherchee
 * \return Le code de la couleur donnee
 */
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

/*!
  * \brief Affiche le menu de selection de la difficulte
  */
 void setDifficulty() {
     ClearScreen();
     int choix = 0;
     while (choix < 1 || choix > 3) {
         cout << Settings.GameStrings["DifficultyChoose"] << endl;
         cout << "1. " << Settings.GameStrings["DifficultyEasy"] << endl;
         cout << "2. " << Settings.GameStrings["DifficultyMedium"] << endl;
         cout << "3. " << Settings.GameStrings["DifficultyHard"] << endl;
         cout << Settings.GameStrings["EnterChoice"] << ":" << endl;
         cin >> choix;
     }
     Settings.difficulty = choix;
     Settings.randomFrequency = 4 - choix;
     ClearScreen();
 }


 /*!
  * \brief Affiche le menu de selection de la langue
  */
 void languageMenu() {
     ClearScreen();
     vector<string> languages = getLanguages();
         cout << Settings.GameStrings["ChangeLanguage"] << " :" << endl;
         int choix = 0;
         string str;
         for (unsigned i = 0; i < languages.size(); ++i) {
             str = languages[i];
             str.erase(str.size() - 4, 4);
             cout << i+1 << ". " << str << endl;
         }
         while (choix < 1 || choix > (int) languages.size())
             cin >> choix;
         setLanguage(languages[choix - 1]);
         ClearScreen();
 }

 /*!
  * \brief Affiche le fichier d'aide
  */
 void showHelp() {
     ClearScreen();
     ifstream stream ("../G1_Brizon_Cartier_DeLaFuente_Roda/Nos_fichiers/langue/" + Settings.GameStrings["HelpFile"]);
     string str;
     while (!stream.eof()) {
         getline(stream, str);
         cout << str << endl;
     }
     char c = 'y';
     cout << Settings.GameStrings["HelpEnd"] << endl;
     while (c != '1') {
         cin >> c;
     }
     ClearScreen();

 }

 /*!
  * \brief Regle l'option selectionnee
  * \param Option a regler
  * \return Si le joueur est pret a jouer ou non
  */
 bool setOption(int & Choix) {
     switch (Choix) {
     case 1:
         setDifficulty();
         return false;
         break;
     case 2:
         languageMenu();
         return false;
         break;
     case 3:
         showHelp();
         return false;
         break;
     case 4:
         return true;
     default:
         return true;
         break;
     }
 }


 /*!
 * \brief Affiche le menu principal
 * \return Si le joueur veut jouer ou non
 */
bool mainMenu() {
    bool ready = false;
    while (!ready) {
        cout << "Menu : " << endl;
        cout << "1. " << Settings.GameStrings["ChangeDifficulty"] << ". " << Settings.GameStrings["Current"] <<": " << Settings.difficulty << endl;
        cout << "2. " << Settings.GameStrings["ChangeLanguage"] << endl;
        cout << "3. " << Settings.GameStrings["Help"] << endl;
        cout << "4. " << Settings.GameStrings["Play"] << endl;
        cout << "5. " << Settings.GameStrings["Quit"] << endl;
        cout << Settings.GameStrings["EnterChoice"] << endl;
        int choix;
        while (true) {
            cin >> choix;
            if (choix == 5)
                return false;
            if (choix <= 0 || choix > 4)
                continue;
            break;
        }
        ready = setOption(choix);
    }
    return true;

}




/* Code copié depuis :
http://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
*/


/*!
 * \brief Remet le mode d'entree par defaut du programme
 */
void reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}


/*!
 * \brief Change le mode d'entree en mode non canonique
 */
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


/*!
 * \brief Démarre le chronomètre
 */
void startTimer() {
    Time = time(NULL);
}

/*!
 * \brief Donne le nombre de secondes ecoulées depuis le debut du chronometre
 * \return Le nombre de seconde ecoulees
 */
unsigned getSecondsElapsed() {
    return ((unsigned) time(NULL) - Time);
}

/*!
 * \brief Donne le temps restant aux joueurs
 * \return Le temps restant
 */
int getTimeLeft() {
    return Settings.endTime - getSecondsElapsed();
}

/*!
 * \brief Met le jeu en pause
 */
void gamePause() {
    stopMusic();
    ClearScreen();
    unsigned elapsedTime = getSecondsElapsed();
    reset_input_mode();
    char c = 'y';
    cout << Settings.GameStrings["PauseDialog"] << endl;
    while (c != '1') {
        cin >> c;
    }
    set_input_mode();
    Time = time(NULL) - elapsedTime;
    ClearScreen();
    playMusic();
}

/*!
 * \brief Fait apparaitre un tresor dans la grille si les conditions sont remplies
 * \param La grille du jeu
 */
void popFood(CMatrix & Mat) {
    if (randomCounter == Settings.randomFrequency) {
        randomCounter = 0;
        random_device rd;
        mt19937 mt(rd());
        uniform_int_distribution<int> vert(0, Mat.size() - 1);
        uniform_int_distribution<int> hor(0, Mat[0].size() - 1);
        CPosition food = pair<int, int> (vert(mt), hor(mt));
        while (food == Player1 || food == Player2) {
            food = pair<int, int> (vert(mt), hor(mt));
        }
        Mat[food.first][food.second] = Settings.food;
    } else
        ++randomCounter;
}

/*!
 * \brief Affiche l'interface principale du jeu
 * \param La grille a afficher
 */
void  ShowMatrix (const CMatrix & Mat) {
    ClearScreen();
    Couleur(KReset);
    cout << Settings.GameStrings["TimeLeft"] << ": " << getTimeLeft() << Settings.GameStrings["P1Score"] << ": " << ScoreJ1 << Settings.GameStrings["P2Score"] << ": " << ScoreJ2 << endl;
    for (unsigned i = 0; i <= Mat[0].size() + 1; ++i) cout << "=";
    cout << endl;
    for (unsigned i = 0; i < Mat.size(); ++i) {
        cout << "|";
        for (unsigned j = 0; j < Mat[i].size(); ++j) {
            if (Mat[i][j] == KEmpty) {
                cout << KEmpty;
            }
            if (i == Player1.first && j == Player1.second) {
                Couleur(getColor(Settings.Colors["ColorPlayer1"]));
                cout << Mat[i][j];
                Couleur(KReset);
            }
            if (i == Player2.first && j == Player2.second) {
                Couleur(getColor(Settings.Colors["ColorPlayer2"]));
                cout << Mat[i][j];
                Couleur(KReset);
            }
            if (Mat[i][j] == Settings.food) {
                Couleur(getColor(Settings.Colors["ColorFood"]));
                cout << Settings.food;
                Couleur(KReset);
            }
        }
        cout << "|" << endl;

    }
    for (unsigned i = 0; i <= Mat[0].size() + 1; ++i) cout << "=";
    cout << endl;
}

/*!
 * \brief Initialise la grille de jeu
 * \param La grille a initialiser
 * \param Le nombre de lignes
 * \param Le nombres de colonnes
 * \param La position du joueur 1
 * \param La position du joueur 2
 */
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

/*!
 * \brief Fonction pour determiner le joueur en fonction d'une position
 * \param Position du joueur inconnu
 * \return Si c'est le joueur 1 ou non
 */
bool isFirstPlayer(CPosition & Pos) {
    cout << Pos.first << " " << Pos.second << endl;
    cout << Player1.first << " " << Player1.second << endl;
    return (Pos == Player1); //Expression booléenne
}

/*!
 * \brief Fonction pour gerer les actions du joueur
 * \param Objet mange par le joueur
 * \param Position de cet objet
 * \param Caractere du joueur
 * \param Position du joueur avant de manger
 * \param Caractere appuye au clavier
 * \return si le joueur a le droit d'effectuer le deplacement
 */
bool checkEat(char & Object, CPosition & PosObject,char & Player, CPosition & PosPlayer, char & Input) {
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
        if (Input == Settings.Controls["J1Up"] || Input == Settings.Controls["J1Down"]
                || Input == Settings.Controls["J1Left"] || Input == Settings.Controls["J1Right"])
            ScoreJ1 = ScoreJ1 + 10;
        else
            ScoreJ2 = ScoreJ2 + 10;
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

/*!
 * \brief Deplace un joueur
 * \param Grille du jeu
 * \param Caractere saisi au clavier
 * \param Position du joueur
 */
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
        if (!checkEat(Mat[Pos.first][Pos.second], Pos, Mat[OldPos.first][OldPos.second], OldPos, Move))
            Pos = OldPos;
    Mat[Pos.first][Pos.second] = Player;
}

/*!
 * \brief Lis la saisie clavier
 * \param Saisie du clavier
 */
void readInput(char & Input) {
    if (Input == Settings.Controls["J1Up"] || Input == Settings.Controls["J1Down"] || Input == Settings.Controls["J1Left"] || Input == Settings.Controls["J1Right"])
        MoveToken(Mat, Input, Player1);
    if (Input == Settings.Controls["J2Up"] || Input == Settings.Controls["J2Down"] || Input == Settings.Controls["J2Left"] || Input == Settings.Controls["J2Right"])
        MoveToken(Mat, Input, Player2);
    if (Input == Settings.Controls["pause"])
        gamePause();
}

/*!
 * \brief Fonction pour avoir la taille de la fenetre
 * \param La variable de taille
 */
void getWindowSize(CTerminalSize & Size) {
    struct winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    Size.first = size.ws_row;
    Size.second = size.ws_col;
}

/*!
 * \brief Fonction appelee a la fin du jeu, pour afficher le gagnant
 * \param Le joueur gagnant
 * \param Le score du gagnant
 * \param Le score du perdant
 * \return Si le joueur veut rejouer ou non
 */
bool win(unsigned Player, unsigned & Score1, unsigned & Score2) {
        cout << Settings.GameStrings["CongratulationsDialog"] << Player << " ! "
             << Settings.GameStrings["WinDialog"] << Score1
             << Settings.GameStrings["FinalScore1"] << Score2 << Settings.GameStrings["FinalScore2"] << endl;
        while (true) {
            cout << "1. " << Settings.GameStrings["BackMenu"] << endl;
            cout << "2. " << Settings.GameStrings["Quit"] << endl;
            int choix;
            cin >> choix;
            ClearScreen();
            if (choix == 1)
                return true;
            if (choix == 2)
                return false;
        }
}

/*!
 * \brief Fonction principale, contient la boucle de jeu
 * \return Si le joueur veut continuer ou non
 */
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
      startTimer();
      playMusic();
      while(Playing) {
            ShowMatrix(Mat);
            if (read (STDIN_FILENO, &Saisie, 1) != 0) // Lit la touche pressée, seulement si il n'y a pas d'erreur
                readInput(Saisie);
            popFood(Mat);
            if (!(getTimeLeft() >= 0))
                break;
      }
      reset_input_mode();
      ClearScreen();
      stopMusic();
      if (ScoreJ1 > ScoreJ2)
         return win(1, ScoreJ1, ScoreJ2);
      else
         return win(2, ScoreJ2, ScoreJ1);
}




/*!
 * \brief Fonction principale
 * \return Le code de sortie du programme
 */
int main()
{
    ClearScreen();
    LoadParams(Settings);
    setDefaults(Settings);
    bool playing = true;
    while(playing) {
        playing = mainMenu();
        if (playing)
            playing = ppal();
    }
    ClearScreen();

    return 0;
}



