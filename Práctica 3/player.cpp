#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>
#include "player.h"
#include "environment.h"
#include <math.h>

using namespace std;

const double masinf=9999999999.0, menosinf=-9999999999.0;


// Constructor
Player::Player(int jug){
    jugador_=jug;
}

// Actualiza el estado del juego para el jugador
void Player::Perceive(const Environment & env){
	actual_=env;
}

double Puntuacion(int jugador, const Environment &estado){
   double suma=0;

   for (int i=0; i<7; i++)
   for (int j=0; j<7; j++){
      if (estado.See_Casilla(i,j)==jugador){
         if (j<3)
            suma += j;
         else
            suma += (6-j);
      }
   }

   return suma;
}


// Funcion de valoracion para testear Poda Alfabeta
double ValoracionTest(const Environment &estado, int jugador){
   int ganador = estado.RevisarTablero();

   if (ganador==jugador)
      return 99999999.0; // Gana el jugador que pide la valoracion
   else if (ganador!=0)
         return -99999999.0; // Pierde el jugador que pide la valoracion
   else if (estado.Get_Casillas_Libres()==0)
         return 0;  // Hay un empate global y se ha rellenado completamente el tablero
   else
         return Puntuacion(jugador,estado);
}

// ------------------- Los tres metodos anteriores no se pueden modificar
int maxFichasColumnas(const Environment &estado, int jugador){
	int numFichasColumnas = 0, maxNumFichasColumnas = -1;

	for(int j = 0; j < 7; j++){
		numFichasColumnas = 0;
		
		if(estado.See_Casilla(6,j) == 0)
			for(int i = 6; i >= 0; i--){
				if(estado.See_Casilla(i,j) == jugador || estado.See_Casilla(i,j) == jugador + 3)
					numFichasColumnas += 1;
				else if(estado.See_Casilla(i,j) != 0)//Es el otro jugador
					break;					
			}

		if(numFichasColumnas > maxNumFichasColumnas)
			maxNumFichasColumnas = numFichasColumnas;
	}

	return maxNumFichasColumnas;
}

double maxFichasFilas(const Environment &estado, int jugador){
	double numFichasFilas = 0.0, maxNumFichasFilas = -1.0;
	int fichasOtroJugador = 0;
	bool filaCompleta = true;

	for(int i = 0; i < 7; i++){
		numFichasFilas = 0.0;
		filaCompleta = true;
		fichasOtroJugador = 0;

		for(int j = 0; j < 7; j++){

			if(estado.See_Casilla(i,j) == '0')
				filaCompleta = false;

			if(estado.See_Casilla(i,j) == ((char) jugador)){
				numFichasFilas += 1.0;
			}
			else{
				if(estado.See_Casilla(i,j) != '0'){
					fichasOtroJugador++;
					if(numFichasFilas > 0)
						if(numFichasFilas > maxNumFichasFilas)
							maxNumFichasFilas = numFichasFilas;
				}
				else
					fichasOtroJugador = 0;
					
				if(numFichasFilas > maxNumFichasFilas && fichasOtroJugador == 0){
					maxNumFichasFilas = numFichasFilas;
					numFichasFilas = 0.0;
				}	
			}

			if(!filaCompleta)
				if(numFichasFilas > maxNumFichasFilas)
					maxNumFichasFilas = numFichasFilas;
		}
			
	}

	return maxNumFichasFilas;
}



// Funcion heuristica (ESTA ES LA QUE TENEIS QUE MODIFICAR)
double Valoracion(const Environment &estado, int jugador){
	int ganador = estado.RevisarTablero();

	if (ganador==jugador)
		return 99999999.0; // Gana el jugador que pide la valoracion
	else if (ganador!=0)
		return -99999999.0; // Pierde el jugador que pide la valoracion
	else if (estado.Get_Casillas_Libres()==0)
		return 0;  // Hay un empate global y se ha rellenado completamente el tablero
	else
		return Puntuacion(jugador,estado);
}





// Esta funcion no se puede usar en la version entregable
// Aparece aqui solo para ILUSTRAR el comportamiento del juego
// ESTO NO IMPLEMENTA NI MINIMAX, NI PODA ALFABETA
void JuegoAleatorio(bool aplicables[], int opciones[], int &j){
   j=0;
   for (int i=0; i<8; i++){
      if (aplicables[i]){
         opciones[j]=i;
         j++;
      }
   }
}

//Revisar
double algoritmoMinimax(Environment actual, int jugador, int profundidad, int profundidadMax, Environment::ActionType  &accion){
   Environment V[8];
   int posAccion, size = actual.GenerateAllMoves(V);
   double valor;

	if(profundidad == profundidadMax || actual.JuegoTerminado())
		return ValoracionTest(actual, jugador);

	if(jugador == actual.JugadorActivo()){
		valor = menosinf;
		double valorMinimax;

		for(int i = 0; i < size; i++){
			valorMinimax = algoritmoMinimax(V[i], jugador, profundidad+1, profundidadMax, accion);
			if(valorMinimax > valor){
				posAccion = i;
				valor = valorMinimax;
			}
		}
	}
	else{
		valor = masinf;
		double valorMinimax;

		for(int i = 0; i < size; i++){
			valorMinimax = algoritmoMinimax(V[i], jugador, profundidad+1, profundidadMax, accion);
			if(valorMinimax < valor){
				posAccion = i;
				valor = valorMinimax;
			}
		}
	}

	accion = static_cast<Environment::ActionType>(V[posAccion].Last_Action(jugador));
	return valor;
}

double Poda_AlfaBeta(Environment actual, int jugador, int profundidad, int profundidadMax, Environment::ActionType &accion, double alpha, double beta){
   Environment V[8];
   int size = actual.GenerateAllMoves(V);
   double valor;

   if(profundidad == profundidadMax || actual.JuegoTerminado())
      return Valoracion(actual, jugador);

   double valorAlfaBeta;

	if(jugador == actual.JugadorActivo()){
		for(int i = 0; i < size; i++){
			valorAlfaBeta = Poda_AlfaBeta(V[i], jugador, profundidad+1, profundidadMax, accion, alpha, beta);
			if(alpha < valorAlfaBeta){
				accion = static_cast<Environment::ActionType>(V[i].Last_Action(jugador));
				valor = alpha = valorAlfaBeta;
			}
				
			if(alpha >= beta)
				return beta;
		}
	}
   	else{
		for(int i = 0; i < size; i++){
			valorAlfaBeta = Poda_AlfaBeta(V[i], jugador, profundidad+1, profundidadMax, accion, alpha, beta);
			if(beta > valorAlfaBeta)
				valor = beta = valorAlfaBeta;

			if(beta <= alpha)
				return alpha;
		}
	}

	return valor;
}


void mostrarMatriz(Environment actual){
	for(int i = 0; i < 7; i++){
		for(int j = 0; j < 7; j++){
			if(actual.See_Casilla(i, j) == 0) cout << 0 << " ";
			else if(actual.See_Casilla(i, j) == 1) cout << 1 << " ";
			else if(actual.See_Casilla(i, j) == 2) cout << 2 << " ";
			else if(actual.See_Casilla(i, j) == 4) cout << 4 << " ";
			else if(actual.See_Casilla(i, j) == 4) cout << 5 << " ";
		}
			
		cout << endl;
	}
		
}

// Invoca el siguiente movimiento del jugador
Environment::ActionType Player::Think(){
	const int PROFUNDIDAD_MINIMAX = 6;  // Umbral maximo de profundidad para el metodo MiniMax
	const int PROFUNDIDAD_ALFABETA = 8; // Umbral maximo de profundidad para la poda Alfa_Beta

	Environment::ActionType accion; // acci�n que se va a devolver
	bool aplicables[8];  // Vector bool usado para obtener las acciones que son aplicables en el estado actual. La interpretacion es
							// aplicables[0]==true si PUT1 es aplicable
							// aplicables[1]==true si PUT2 es aplicable
							// aplicables[2]==true si PUT3 es aplicable
							// aplicables[3]==true si PUT4 es aplicable
							// aplicables[4]==true si PUT5 es aplicable
							// aplicables[5]==true si PUT6 es aplicable
							// aplicables[6]==true si PUT7 es aplicable
							// aplicables[7]==true si BOOM es aplicable



	double valor; // Almacena el valor con el que se etiqueta el estado tras el proceso de busqueda.
	double alpha = menosinf, beta = masinf; // Cotas de la poda AlfaBeta

	int n_act; //Acciones posibles en el estado actual


	n_act = actual_.possible_actions(aplicables); // Obtengo las acciones aplicables al estado actual en "aplicables"
	int opciones[10];

	// Muestra por la consola las acciones aplicable para el jugador activo
	//actual_.PintaTablero();
	cout << " Acciones aplicables ";
	(jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
	for (int t=0; t<8; t++)
	if (aplicables[t])
		cout << " " << actual_.ActionStr( static_cast< Environment::ActionType > (t)  );
	cout << endl;


	//--------------------- COMENTAR Desde aqui
	//  cout << "\n\t";
	//  int n_opciones=0;
	//  JuegoAleatorio(aplicables, opciones, n_opciones);

	//  if (n_act==0){
	//    (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
	//    cout << " No puede realizar ninguna accion!!!\n";
	//    //accion = Environment::actIDLE;
	//  }
	//  else if (n_act==1){
	//         (jugador_==1) ? cout << "Verde: " : cout << "Azul: ";
	//          cout << " Solo se puede realizar la accion "
	//               << actual_.ActionStr( static_cast< Environment::ActionType > (opciones[0])  ) << endl;
	//          accion = static_cast< Environment::ActionType > (opciones[0]);

	//       }
	//       else { // Hay que elegir entre varias posibles acciones
	//          int aleatorio = rand()%n_opciones;
	//          cout << " -> " << actual_.ActionStr( static_cast< Environment::ActionType > (opciones[aleatorio])  ) << endl;
	//          accion = static_cast< Environment::ActionType > (opciones[aleatorio]);
	//       }

	//--------------------- COMENTAR Hasta aqui


	//--------------------- AQUI EMPIEZA LA PARTE A REALIZAR POR EL ALUMNO ------------------------------------------------
	// Opcion: Poda AlfaBeta
	// NOTA: La parametrizacion es solo orientativa
	// cout << "VALORACION COLUMNAS " << maxFichasColumnas(actual_, jugador_) << endl;

	valor = Poda_AlfaBeta(actual_, jugador_, 0, PROFUNDIDAD_ALFABETA, accion, alpha, beta);
	cout << "Valor AlfaBeta: " << valor << "  Accion: " << actual_.ActionStr(accion) << endl;

	// cout << "MATRIZ ****************" << endl;
	// mostrarMatriz(actual_);

	// cout << "COLUMNAS OCUPADAS POR JUGADOR " << jugador_ << ": " << maxFichasColumnas(actual_, jugador_) << " ****************" << endl;


	// valor = algoritmoMinimax(actual_, jugador_, 0, PROFUNDIDAD_MINIMAX, accion);
	// cout << "Valor MiniMax: " << valor << "  Accion: " << actual_.ActionStr(accion) << endl;

   return accion;
}

