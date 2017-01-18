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
#include "sound.h"

using namespace std;


//Temporaire, a changer avec les fichiers de config
const char KUP ('z');
const char KDOWN ('s');
const char KLEFT ('q');
const char KRIGHT ('d');
const char KUP2 ('o');
const char KDOWN2 ('l');
const char KLEFT2 ('k');
const char KRIGHT2 ('m');

const char KPAUSE ('g');

const char KFOOD ('#');
//Fin des valeurs temporaires

const char KPlayerUp ('^');
const char KPlayerDown ('v');
const char KPlayerLeft ('<');
const char KPlayerRight ('>');


void ClearScreen ()
{
    cout << "\033[H\033[2J";
}

const string KReset   ("0");
const string KNoir    ("30");
const string KRouge   ("31");
const string KVert    ("32");
const string KJaune   ("33");
const string KBleu    ("34");
const string KMAgenta ("35");
const string KCyan    ("36");


void Couleur (const string & coul)
{
    cout << "\033[" << coul <<"m";
}

typedef vector <char> CVLine; // un type représentant une ligne de la grille
typedef vector <CVLine> CMatrix; // un type représentant la grille
typedef pair   <unsigned, unsigned> CPosition; // un type représentant une coordonnée dans la grille

typedef pair <unsigned, unsigned> CTerminalSize; // Un type qui représente la taille de la fenetre

//Caractères par défaut
const char KTokenPlayer1 = 'X';
const char KTokenPlayer2 = 'O';
const char KEmpty        = ' ';

//Stockage des paramètres du Terminal, pour changer le mode entre canonique et non canonique
struct termios saved_attributes;


//Variables utilisées pendant le jeu
CMatrix Mat;
CPosition Player1;
CPosition Player2;
unsigned ScoreJ1;
unsigned ScoreJ2;
int randomFrequency = 1;
int randomCounter = 0;

//Variables qui gardent le temps
unsigned long Time;
unsigned EndTime;



pair <unsigned, string> displayMenu( vector<string> & Items) {
    cout << "Menu : " << endl;
    unsigned i;
    for (i = 1; i <= Items.size(); ++i)
        cout << i << " : " << Items[i - 1] << endl;
    cout << "Entrez votre choix :" << endl;
    int choix;
    cin >> choix;
    while (true)
        if (choix <= 0 || choix > Items.size())
            return  pair<unsigned, string> (i - 1, Items[i - 1]);
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

vector<string> getDirectoryContents() {
    vector<string> Langues;
    /* Source : http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c */
   DIR  *d;
  struct dirent *dir;
  d = opendir("./langue");
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

void startTimer() {
    Time = clock();
}

unsigned getSecondsElapsed() {
    return ((unsigned) clock() - Time) / CLOCKS_PER_SEC;
}

int getTimeLeft() {
    return EndTime - getSecondsElapsed();
}

void gamePause() {
    ClearScreen();
    reset_input_mode();
    char c = 'y';
    cout << "Jeu en pause. Appuyez sur 1 puis entrée pour reprendre." << endl;
    while (c != '1') {
        cin >> c;
    }
    set_input_mode();
    ClearScreen();
}

void popFood(CMatrix & Mat) {
    if (randomCounter == randomFrequency) {
        randomCounter = 0;
        random_device rd;
        mt19937 mt(rd());
        uniform_int_distribution<int> vert(1, Mat.size());
        uniform_int_distribution<int> hor(1, Mat[0].size());
        CPosition food = pair<int, int> (vert(mt), hor(mt));
        while (food == Player1 || food == Player2) {
            food = pair<int, int> (vert(mt), hor(mt));
        }
        Mat[food.first][food.second] = KFOOD;
    } else
        ++randomCounter;
}


void  ShowMatrix (const CMatrix & Mat) {
    ClearScreen();
    Couleur(KReset);
    cout << "Temps restant : " << getTimeLeft() << " Score J1: " << ScoreJ1 << " Score J2: " << ScoreJ2;
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
            if (Mat[i][j] == KFOOD) {
                Couleur(KBleu);
                cout << KFOOD;
                Couleur(KReset);
            }
        }
        cout << "|" << endl;

    }
    for (unsigned i = 0; i <= Mat[0].size() + 1; ++i) cout << "=";
    cout << endl;
}



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
    Mat[PosPlayer1.first][PosPlayer1.second] = KTokenPlayer1;
    PosPlayer2 = {NbLine - 1, NbColumn - 1};
    Mat[PosPlayer2.first][PosPlayer2.second] = KTokenPlayer2;
}



bool isFirstPlayer(CPosition & Pos) {
    return (Pos == Player1); //Expression booléenne
}

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
      case KFOOD:
        if (isFirstPlayer(PosPlayer))
            ++ScoreJ1;
        else
            ++ScoreJ2;
        break;
    default:  // Pas un joueur
        break;
    }

    return true;
}

void MoveToken (CMatrix & Mat, char Move, CPosition  & Pos) {
    CPosition OldPos = Pos;
    char Player;
    Mat[Pos.first][Pos.second] = KEmpty;
    switch (Move) {
    case KUP:
    case KUP2 : if (Pos.first > 0) Pos.first = Pos.first - 1;
        Player = KPlayerUp;
        break;
    case KLEFT:
    case KLEFT2 : if (Pos.second) Pos.second = Pos.second - 1;
        Player = KPlayerLeft;
        break;
    case KRIGHT:
    case KRIGHT2 : if (Pos.second < Mat[0].size() - 1) Pos.second = Pos.second + 1;
        Player = KPlayerRight;
        break;
    case KDOWN:
    case KDOWN2 : if (Pos.first < Mat.size() - 1) Pos.first = Pos.first + 1;
        Player = KPlayerDown;
        break;
    default: //Ne rien faire
        break;
    }
    if (Mat[Pos.first][Pos.second] != KEmpty)
        if (!checkEat(Mat[Pos.first][Pos.second], Pos, Mat[OldPos.first][OldPos.second], OldPos))
            Pos = OldPos;
    Mat[Pos.first][Pos.second] = Player;
}


void readInput(char & Input) {
    switch (Input) {
    case KUP:
    case KDOWN:
    case KLEFT:
    case KRIGHT:
        MoveToken(Mat, Input, Player1);
        break;
    case KUP2:
    case KDOWN2:
    case KLEFT2:
    case KRIGHT2:
        MoveToken(Mat, Input, Player2);
        break;
    case KPAUSE:
        gamePause();
        break;
    default:
        // Ne rien faire
        break;
    }
}


//Fonction qui donne la taille de la fenêtre
void getWindowSize(CTerminalSize & Size) {
    struct winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    Size.first = size.ws_row;
    Size.second = size.ws_col;
}


void win(unsigned Player, unsigned & Score1, unsigned & Score2) {
        cout << "Félicitations au Joueur " << Player << " ! Il gagne avec " << Score1 << " points contre " << Score2 << "points." << endl;
}

int ppal () {
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
            if (!(getTimeLeft() >= 0))
                break;
      }
      reset_input_mode();
      if (ScoreJ1 > ScoreJ2)
          win(1, ScoreJ1, ScoreJ2);
      else
          win(2, ScoreJ2, ScoreJ1);

      return 0;
}





int main(int argc, char *argv[])
{
    //Menu ici, le jeu utilisera les reglages stockés dans des variables globales

    ppal();

    return 0;
}



