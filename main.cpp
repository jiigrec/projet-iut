#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>


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

CPosition Player1;
CPosition Player2;
int ScoreJ1;
int ScoreJ2;



pair <unsigned, string> displayMenu( vector<string> & Items) {

    //Cette fonction va afficher un menu avec tous les éléments du vecteur. Apres le choix, la fonction retourne une paire
    //avec le numero du choix et la chaine de caractère du choix



}





vector<string> getDirectoryContents(string & Directory) {

    //Donne la liste des fichiers contenus dans un dossier et les donne dans un vcteur.

   //Regarder sur internet pour celle la. C'est pas inné ni dans les TP

}





void  ShowMatrix (const CMatrix & Mat) {
    ClearScreen();
    Couleur(KReset);
    for (unsigned i = 0; i <= Mat[0].size() + 1; ++i) cout << "=";
    cout << endl;
    for (unsigned i = 0; i < Mat.size(); ++i) {
        cout << "|";
        for (unsigned j = 0; j < Mat[i].size(); ++j) {
            if (Mat[i][j] == KEmpty) {
                cout << KEmpty;
            }
            if (Mat[i][j] == KTokenPlayer1) {
                Couleur(KVert);
                cout << KTokenPlayer1;
                Couleur(KReset);
            }
            if (Mat[i][j] == KTokenPlayer2) {
                Couleur(KRouge);
                cout << KTokenPlayer2;
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
            ++ScoreJ1;
        else
            ++ScoreJ2;


        break;
      case KFOOD:

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





void getWindowSize(CTerminalSize & Size) {
    struct winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    /* size.ws_row is the number of rows, size.ws_col is the number of columns. */
    Size.first = size.ws_row;
    Size.second = size.ws_col;
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


unsigned getMaxPlays(CMatrix & Mat) {
    return ((Mat.size() * Mat[0].size()) / 10);
}

int ppal () {
      bool Playing = true;
      CMatrix Mat;
      unsigned Cpt = 0;
      CTerminalSize WindowSize;
      getWindowSize(WindowSize);
      InitMat(Mat, WindowSize.first - 5,WindowSize.second - 2,Player1,Player2);
      unsigned MaxPlays = getMaxPlays(Mat);
      ShowMatrix(Mat);
      char Saisie;
      bool Player = true;
      bool Win = false;
      set_input_mode();
      ScoreJ1 = 0;
      ScoreJ2 = 0;
      while(Playing) {
            cout << "Coups: " << MaxPlays << ", joués : " << Cpt << endl;
            if (Player) { cout << "Au tour du ";
            Couleur(KVert);
            cout << "joueur 1 ! " << endl; }
            if (!Player) { cout << "Au tour du ";
            Couleur(KRouge);
            cout << "joueur 2 ! " << endl; }
            Couleur(KReset);
            read (STDIN_FILENO, &Saisie, 1); //Lit la touche pressée.
            if (Player) MoveToken(Mat, Saisie, Player1);
            if (!Player) MoveToken(Mat, Saisie, Player2);
            ++Cpt;
            Player = !(Player);
            ShowMatrix(Mat);
            /* Refaire cette fonction pour les conditions de victoire. */
            if (Player1 == Player2) {
                Win = true;
                break;
            }
            if (Cpt == MaxPlays) break;
      }
      reset_input_mode();
      if (Win) {
          cout << "Félicitations ! " << endl;
          if (!Player) cout << "Le joueur 1 a gagné." << endl;
          if (Player) cout << "Le joueur 2 a gagné." << endl;
      } else {
          cout << "Match nul." << endl;
      }
      return 0;
}





int main(int argc, char *argv[])
{
    //Menu ici, le jeu utilisera les reglages stockés dans des variables globales

    ppal();

    return 0;
}


/* Exemple de menu pour les noobs :
 *
 *  cout << "menu :" << endl << "1. Pates " << endl << "2. PIzza " << endl ;
    int choix;
    cin >> choix;
    switch (choix) {
    case 1:
        cout << "Vs avez choisi des pates" << endl;
        break;
    case 2:
        cout << "Vous avez choisi la pizza." << endl;
    default:
        cout << "Ce choix n'est pas disponible !" <<endl;
        break;
    }


    */


