#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>


using namespace std;


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


void MoveToken (CMatrix & Mat, char Move, CPosition  & Pos) {
    char Player = Mat[Pos.first][Pos.second];
    Mat[Pos.first][Pos.second] = KEmpty;
    switch (Move) {
    case 'a':
        if (Pos.first > 0) Pos.first = Pos.first - 1;
        if (Pos.second > 0) Pos.second = Pos.second - 1;
        break;
    case 'z' : if (Pos.first > 0) Pos.first = Pos.first - 1;
        break;
    case 'e' : if (Pos.first > 0 ) Pos.first = Pos.first - 1;
        if (Pos.second < Mat[0].size() - 1) Pos.second = Pos.second + 1;
        break;
    case 'q' : if (Pos.second) Pos.second = Pos.second - 1;
        break;
    case 's' : //Rien faire (le joueur ne bouge pas)
        break;
    case 'd' : if (Pos.second < Mat[0].size() - 1) Pos.second = Pos.second + 1;
        break;
    case 'w' : if (Pos.first < Mat.size() - 1) Pos.first = Pos.first + 1;
        if (Pos.second) Pos.second = Pos.second - 1;
        break;
    case 'x' : if (Pos.first < Mat.size() - 1) Pos.first = Pos.first + 1;
        break;
    case 'c' : if (Pos.first < Mat.size() - 1) Pos.first = Pos.first + 1;
        if (Pos.second < Mat[0].size() - 1) Pos.second = Pos.second + 1;
        break;
    default: //Ne rien faire
        break;
    }
    Mat[Pos.first][Pos.second] = Player;
}


void getWindowSize(CTerminalSize & Size) {
    struct winsize size;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    /* size.ws_row is the number of rows, size.ws_col is the number of columns. */
    Size.first = size.ws_row;
    Size.second = size.ws_col;
}


//http://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html
void set_input_mode (void)
{
  struct termios tattr;

  /* Make sure stdin is a terminal. */
  if (!isatty (STDIN_FILENO))
  {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
  }

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 0;
  tattr.c_cc[VTIME] = 3;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}


unsigned getMaxPlays(CMatrix & Mat) {
    return ((Mat.size() * Mat[0].size()) / 10);
}

int ppal () {
      bool Playing = true;
      CMatrix Mat;
      CPosition Player1;
      CPosition Player2;
      unsigned Cpt = 0;
      CTerminalSize WindowSize;
      getWindowSize(WindowSize);
      InitMat(Mat, WindowSize.first - 5,WindowSize.second - 2,Player1,Player2);
      unsigned MaxPlays = getMaxPlays(Mat);
      ShowMatrix(Mat);
      char Saisie;
      bool Player = true;
      bool Win = false;
      while(Playing) {
            cout << "Coups: " << MaxPlays << ", joués : " << Cpt << endl;
            if (Player) { cout << "Au tour du ";
            Couleur(KVert);
            cout << "joueur 1 ! " << endl; }
            if (!Player) { cout << "Au tour du ";
            Couleur(KRouge);
            cout << "joueur 2 ! " << endl; }
            Couleur(KReset);
            cin >> Saisie;
            if (Player) MoveToken(Mat, Saisie, Player1);
            if (!Player) MoveToken(Mat, Saisie, Player2);
            ++Cpt;
            Player = !(Player);
            ShowMatrix(Mat);
            if (Player1 == Player2) {
                Win = true;
                break;
            }
            if (Cpt == MaxPlays) break;
      }
      if (Win) {
          cout << "Félicitations ! " << endl;
          if (!Player) cout << "Le joueur 1 a gagné." << endl;
          if (Player) cout << "Le joueur 2 a gagné." << endl;
      } else {
          cout << "Match nul." << endl;
      }
      return 0; //TODO: changer ca
}





int main(int argc, char *argv[])
{
    ppal();

    return 0;
}
