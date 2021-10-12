#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>

// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores) {
	instantesConsumidos++;
	Action accion = actIDLE;

	//Actualizo la variable actual
	actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;

	actualizarEstado(actual);

	// Capturo los destinos
	objetivos.clear();
	for(int i=0; i<sensores.num_destinos; i++){
		estado aux;
		aux.fila = sensores.destino[2*i];
		aux.columna = sensores.destino[2*i+1];
		objetivos.push_back(aux);
	}
			
	//Si no hay plan construyo uno
	if(!hayPlan)
		hayPlan = pathFinding (sensores.nivel, actual, objetivosPendientes, plan);

	if(sensores.nivel == 4){

		//Pinto el mapa con la visión
		pintarMapa(sensores);

		if(((sensores.bateria < 2980 && instantesConsumidos < 2500) || (sensores.bateria < 250 && instantesConsumidos >= 2800)) && mapaResultado[actual.fila][actual.columna] == 'X'){
			hayPlan = true;
			return actIDLE;
		}

		if(necesitaRecargar(sensores) && !bateriasEncontradas.empty()){
			if(instantesConsumidos < 2500 || (instantesConsumidos >= 2500 && sensores.bateria <= 500)){
				ordenarLista(bateriasEncontradas);
				if(!buscar(bateriasEncontradas.front(), objetivosPendientes))
					objetivosPendientes.push_back(bateriasEncontradas.front());
			}			
		}

		if(plan.front() == actFORWARD && sensores.superficie[2] == 'a')
			return actIDLE;


		//Si está en un objetivo
		if(actual.fila == objetivosPendientes.front().fila && actual.columna == objetivosPendientes.front().columna){
			//Elimino el primer objetivo puesto que es en el que estoy
			objetivosPendientes.pop_front();
			//Vuelvo a ordenar para ir a los objetivos más cercanos a la posicion actual
			ordenarLista(objetivosPendientes);
		}
			
		//Cada dos pasos recalcula una nueva ruta o hay un obstaculo delante
		if(pasosDados % 2 == 0 || HayObstaculoDelante(actual)){
			//Los vuelvo a ordenar por si hay otro más cerca cuando vaya a recalcular la ruta
			hayPlan = pathFinding(sensores.nivel, actual, objetivosPendientes, plan);
		}
		
		pasosDados++;
	}

	Action sigAccion;
	if(hayPlan && plan.size() > 0){ //Hay un plan no vacío
		sigAccion = plan.front(); //Tomo la siguiente acción del plan
		plan.erase(plan.begin()); //Elimino la acción del plan
	}
	else
		cout << "No se pudo encontrar un plan" << endl;
		
    return sigAccion;
}

// Llama al algoritmo de busqueda que se usara en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const list<estado> &destino, list<Action> &plan){
	estado un_objetivo;
	un_objetivo = objetivos.front();
	cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;
	switch (level){
		case 0: cout << "Demo\n";
			return pathFinding_Profundidad(origen,un_objetivo,plan);
			break;

		case 1: cout << "Optimo numero de acciones\n";
			return pathFinding_Anchura(origen,un_objetivo,plan);
			break;
			
		case 2: cout << "Optimo en coste 1 Objetivo\n";
			return pathFinding_MenorConsumo(origen,un_objetivo,plan);
			break;
		case 3: cout << "Optimo en coste 3 Objetivos\n";
			cout << "No implementado" << endl;
			break;
		case 4: cout << "Algoritmo de busqueda usado en el reto\n";
			if(objetivosPendientes.empty()){
				objetivosPendientes = objetivos;
				ordenarLista(objetivosPendientes);
			}
			un_objetivo = objetivosPendientes.front();
			return pathFinding_AEstrella(origen, un_objetivo, plan);
			break;
	}
	return false;
}

//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el codigo en caracter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parametro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else
	  return true;
}

struct nodo{
	estado st;
	list<Action> secuencia;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion) or
		  (a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.zapatillas > n.zapatillas) or
		  (a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.zapatillas == n.zapatillas and a.bikini > n.bikini))
			return true;
		else
			return false;
	}
};

struct ComparaConsumo{
	bool operator() (const nodo &n1, const nodo &n2){
		return n1.st.costeAcumulado > n2.st.costeAcumulado;
	}
};

struct ComparaEstimacion{
	bool operator() (const nodo &n1, const nodo &n2){
		return n1.st.estimacionCosteTotal > n2.st.estimacionCosteTotal;
	}
};



// Implementación de la busqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> Cerrados; // Lista de Cerrados
	stack<nodo> Abiertos;				 // Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

  	while (!Abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
			if (Cerrados.find(hijoForward.st) == Cerrados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
			current = Abiertos.top();
		
	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else 
		cout << "No encontrado plan\n";

	return false;
}

// Implementación de la busqueda en anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> Cerrados; // Lista de Cerrados
	queue<nodo> Abiertos;				 // Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

  	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)){
		
		//Saco de abiertos el nodo de menor peso
		Abiertos.pop();

		//Si el nodo no esta explorado
		if(Cerrados.find(current.st) == Cerrados.end()){
			Cerrados.insert(current.st);

			// Generar descendiente de girar a la derecha
			nodo hijoTurnR = current;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
			//Si no esta en explorados lo vuelvo a introducir en abiertos
			if (Cerrados.find(hijoTurnR.st) == Cerrados.end()){
				hijoTurnR.secuencia.push_back(actTURN_R);
				Abiertos.push(hijoTurnR);
			}

			// Generar descendiente de girar a la izquierda
			nodo hijoTurnL = current;
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
			//Si no esta en explorados lo vuelvo a introducir en abiertos
			if (Cerrados.find(hijoTurnL.st) == Cerrados.end()){
				hijoTurnL.secuencia.push_back(actTURN_L);
				Abiertos.push(hijoTurnL);
			}

			// Generar descendiente de avanzar
			nodo hijoForward = current;
			//Si no hay obstaculos delante
			if (!HayObstaculoDelante(hijoForward.st))
				//Si no esta en explorados lo vuelvo a introducir en abiertos
				if (Cerrados.find(hijoForward.st) == Cerrados.end()){
					hijoForward.secuencia.push_back(actFORWARD);
					Abiertos.push(hijoForward);
				}
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
			current = Abiertos.front();
		
	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else 
		cout << "No encontrado plan\n";

	return false;
}

// Implementación de la busqueda para un consumo de batería mínimo.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_MenorConsumo(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> Cerrados; // Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaConsumo> Abiertos;				 // Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

  	while (!Abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		//Saco de abiertos el nodo de menor peso
		Abiertos.pop();

		//Si el nodo no esta explorado
		if(Cerrados.find(current.st) == Cerrados.end()){
			//Inserto en cerrados (explorados) el estado del nodo actual
			Cerrados.insert(current.st);

			// Generar descendiente de girar a la derecha
			nodo hijoTurnR = current;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
			//Si no esta en explorados lo vuelvo a introducir en abiertos
			if (Cerrados.find(hijoTurnR.st) == Cerrados.end()){
				//Calculo el coste acumulado (coste acumulado de current + coste casilla hijoTurnR)
				calcularConsumo(current.st, hijoTurnR.st, actTURN_R);
				hijoTurnR.secuencia.push_back(actTURN_R);
				Abiertos.push(hijoTurnR);
			}

			// Generar descendiente de girar a la izquierda
			nodo hijoTurnL = current;
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
			//Si no esta en explorados lo vuelvo a introducir en abiertos
			if (Cerrados.find(hijoTurnL.st) == Cerrados.end()){
				//Calculo el coste acumulado (coste acumulado de current + coste nodo hijoTurnL)
				calcularConsumo(current.st, hijoTurnL.st, actTURN_L);
				hijoTurnL.secuencia.push_back(actTURN_L);
				Abiertos.push(hijoTurnL);
			}

			// Generar descendiente de avanzar
			nodo hijoForward = current;
			actualizarEstado(hijoForward.st); //Si la casilla es de zapatillas o bikini actualizo las variables del hijo
			//Si no hay obstaculos delante
			if (!HayObstaculoDelante(hijoForward.st))
				//Si no esta en explorados lo vuelvo a introducir en abiertos
				if (Cerrados.find(hijoForward.st) == Cerrados.end()){
					//Calculo el coste acumulado (coste acumulado de current + coste nodo hijoForward)
					calcularConsumo(current.st, hijoForward.st, actFORWARD);
					hijoForward.secuencia.push_back(actFORWARD);
					Abiertos.push(hijoForward);
				}
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
			current = Abiertos.top();
	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else 
		cout << "No encontrado plan\n";

	return false;
}

// Implementación de la busqueda para el nivel 4.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_AEstrella(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> Cerrados; // Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaEstimacion> Abiertos;// Lista de Abiertos
  	nodo current;
	current.st = origen;
	actualizarDistancia(current.st, destino);
	actualizarEstimacionCoste(current.st);
	current.secuencia.empty();

	Abiertos.push(current);

  	while (!Abiertos.empty() && (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		//Saco de abiertos el nodo de menor peso
		Abiertos.pop();

		//Si el nodo no esta explorado
		if(Cerrados.find(current.st) == Cerrados.end()){
			//Inserto en cerrados (explorados) el estado del nodo actual
			Cerrados.insert(current.st);
			// Generar descendiente de girar a la derecha
			nodo hijoTurnR = current;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
			//Calculo el coste acumulado (coste acumulado de current + coste casilla hijoTurnR)
			calcularConsumo(current.st, hijoTurnR.st, actTURN_R);
			actualizarDistancia(hijoTurnR.st, destino);
			actualizarEstimacionCoste(hijoTurnR.st);
			hijoTurnR.secuencia.push_back(actTURN_R);
			//Si no esta en explorados lo vuelvo a introducir en abiertos
			if (Cerrados.find(hijoTurnR.st) == Cerrados.end() ){
				Abiertos.push(hijoTurnR);
			}
			else{
				set<estado,ComparaEstados>::iterator it = Cerrados.find(hijoTurnR.st);
				if((*it).estimacionCosteTotal > hijoTurnR.st.estimacionCosteTotal){
					Cerrados.erase(it);
					Cerrados.insert(hijoTurnR.st);
				}
			}

			// Generar descendiente de girar a la izquierda
			nodo hijoTurnL = current;
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
			//Calculo el coste acumulado (coste acumulado de current + coste nodo hijoTurnL)
				calcularConsumo(current.st, hijoTurnL.st, actTURN_L);
				actualizarDistancia(hijoTurnL.st, destino);
				actualizarEstimacionCoste(hijoTurnL.st);
				hijoTurnL.secuencia.push_back(actTURN_L);
			//Si no esta en explorados lo vuelvo a introducir en abiertos
			if (Cerrados.find(hijoTurnL.st) == Cerrados.end()){
				Abiertos.push(hijoTurnL);
			}
			else{
				set<estado,ComparaEstados>::iterator it = Cerrados.find(hijoTurnL.st);
				if((*it).estimacionCosteTotal > hijoTurnL.st.estimacionCosteTotal){
					Cerrados.erase(it);
					Cerrados.insert(hijoTurnL.st);
				}
			}

			// Generar descendiente de avanzar
			nodo hijoForward = current;
			actualizarEstado(hijoForward.st); //Si la casilla es de zapatillas o bikini actualizo las variables del hijo
			//Si no hay obstaculos delante
			if (!HayObstaculoDelante(hijoForward.st)){
				//Calculo el coste acumulado (coste acumulado de current + coste nodo hijoForward)
					calcularConsumo(current.st, hijoForward.st, actFORWARD);
					actualizarDistancia(hijoForward.st, destino);
					actualizarEstimacionCoste(hijoForward.st);
					hijoForward.secuencia.push_back(actFORWARD);
				//Si no esta en explorados lo vuelvo a introducir en abiertos
				if (Cerrados.find(hijoForward.st) == Cerrados.end()){
					Abiertos.push(hijoForward);
				}
				else{
					set<estado,ComparaEstados>::iterator it = Cerrados.find(hijoForward.st);
					if((*it).estimacionCosteTotal > hijoForward.st.estimacionCosteTotal){
						Cerrados.erase(it);
						Cerrados.insert(hijoForward.st);
					}
				}
			}
				
				
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
			current = Abiertos.top();

	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila==destino.fila or current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else 
		cout << "No encontrado plan\n";

	return false;
}

// Sacar por la consola la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

// Funcion auxiliar para poner a 0 todas las casillas de una matriz
void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}

int ComportamientoJugador::calcularCosteCasilla(estado &st, const vector<vector<unsigned char>> &mapa, Action accion){
	int costeCasilla = 0;

	//Compruebo que acción se va a tomar
	switch (accion){
		//Si la accion es ir hacia delante
		case actFORWARD:
			//Compruebo en que casilla me encuentro
			switch (mapa[st.fila][st.columna]){
				//El terreno es de tipo Arboles
				case 'B':
					if(st.zapatillas)
						costeCasilla = 15;
					else
						costeCasilla = 100;
				break;

				//El terreno es de tipo Agua
				case 'A':
					if(st.bikini)
						costeCasilla = 10;
					else
						costeCasilla = 200;
				break;

				//El terreno es de tipo Suelo Arenoso
				case 'T':
					costeCasilla = 2;
				break;

				//El terreno es de otro tipo
				default:
					costeCasilla = 1;
				break;
			}
		break;
		//Si la acción es girar
		case actTURN_R: case actTURN_L:
			//Compruebo en que casilla me encuentro
			switch (mapa[st.fila][st.columna]){
				//El terreno es de tipo Arboles
				case 'B':
					if(st.zapatillas)
						costeCasilla = 1;
					else
						costeCasilla = 3;
				break;

				//El terreno es de tipo Agua
				case 'A':
					if(st.bikini)
						costeCasilla = 5;
					else
						costeCasilla = 500;
				break;

				//El terreno es de tipo Suelo Arenoso
				case 'T':
					costeCasilla = 2;
				break;

				//El terreno es de otro tipo
				default:
					costeCasilla = 1;
				break;
			}
		break;

		//Accion = actIDLE
		default:
			costeCasilla = 0;
		break;
	}

	return costeCasilla;
}

void ComportamientoJugador::calcularConsumo(const estado &padre, estado &hijo, Action accion){
	int costeCasilla = calcularCosteCasilla(hijo, mapaResultado, accion);
	hijo.costeAcumulado = costeCasilla + padre.costeAcumulado;
}

void ComportamientoJugador::actualizarEstado(estado &st){
	//Pongo al estado las zapatillas
	if(mapaResultado[st.fila][st.columna] == 'D'){
		st.bikini = false;
		st.zapatillas = true;
	}
	//Pongo al estado el bikini
	else if(mapaResultado[st.fila][st.columna] == 'K'){
			st.zapatillas = false;
			st.bikini = true;
	}
}

//Actualiza la distancia que hay desde la posicion actual hasta un destino
void ComportamientoJugador::actualizarDistancia(estado &st, const estado &destino){
	st.distanciaDestino = abs(st.fila - destino.fila) + abs(st.columna - destino.columna);
}

//Actualiza la distancia la variable estimacionCosteTotal del estado pasado como argumento 
void ComportamientoJugador::actualizarEstimacionCoste(estado &st){
	st.estimacionCosteTotal = st.distanciaDestino + st.costeAcumulado;
}

//Pinta el mapa y si ve una bateria, la guarda en bateriasEncontradas si no estaba ya introducida
void ComportamientoJugador::pintarMapa(Sensores sensor){
	int posTerreno = 1;
	estado bateria;

	mapaResultado[sensor.posF][sensor.posC] = sensor.terreno[0];

	if(sensor.terreno[0] == 'X'){
		bateria.fila = sensor.posF;
		bateria.columna = sensor.posC;
		if(!buscar(bateria, bateriasEncontradas))
			bateriasEncontradas.push_back(bateria);
	}	

	//Compruebo la orientación
	switch(sensor.sentido){
		//La orientación es hacia el norte
		case norte:

			for(int i = -1; i <= 1; i++){
				mapaResultado[sensor.posF-1][sensor.posC+i] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF-1;
					bateria.columna = sensor.posC+i;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = -2; i <= 2; i++){
				mapaResultado[sensor.posF-2][sensor.posC+i] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF-2;
					bateria.columna = sensor.posC+i;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = -3; i <= 3; i++){
				mapaResultado[sensor.posF-3][sensor.posC+i] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF-3;
					bateria.columna = sensor.posC+i;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}
		break;
		//La orientación es hacia el sur
		case sur:

			for(int i = 1; i >= -1; i--){
				mapaResultado[sensor.posF+1][sensor.posC+i] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+1;
					bateria.columna = sensor.posC+i;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = 2; i >= -2; i--){
				mapaResultado[sensor.posF+2][sensor.posC+i] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+2;
					bateria.columna = sensor.posC+i;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = 3; i >= -3; i--){
				mapaResultado[sensor.posF+3][sensor.posC+i] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+3;
					bateria.columna = sensor.posC+i;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}			
		break;
		//La orientación es hacia el este
		case este:

			for(int i = -1; i <= 1; i++){
				mapaResultado[sensor.posF+i][sensor.posC+1] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+i;
					bateria.columna = sensor.posC+1;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = -2; i <= 2; i++){
				mapaResultado[sensor.posF+i][sensor.posC+2] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+i;
					bateria.columna = sensor.posC+2;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = -3; i <= 3; i++){
				mapaResultado[sensor.posF+i][sensor.posC+3] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+i;
					bateria.columna = sensor.posC+3;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}
		break;
		//La orientación es hacia el oeste
		case oeste:

			for(int i = 1; i >= -1; i--){
				mapaResultado[sensor.posF+i][sensor.posC-1] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+i;
					bateria.columna = sensor.posC-1;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = 2; i >= -2; i--){
				mapaResultado[sensor.posF+i][sensor.posC-2] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+i;
					bateria.columna = sensor.posC-2;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}

			for(int i = 3; i >= -3; i--){
				mapaResultado[sensor.posF+i][sensor.posC-3] = sensor.terreno[posTerreno];
				if(sensor.terreno[posTerreno] == 'X'){
					bateria.fila = sensor.posF+i;
					bateria.columna = sensor.posC-3;
					if(!buscar(bateria, bateriasEncontradas))
						bateriasEncontradas.push_back(bateria);
				}
				posTerreno++;
			}
		break;
	}

}

//Funcion que ordena una lista de estado de menor a mayor distancia respecto a la posicion actual
void ComportamientoJugador::ordenarLista(list<estado> &lista){

	list<estado> auxList;
	estado menor, aux;
	list<estado>::iterator it, auxIt;

	while(!lista.empty()){

		menor = lista.front();
		actualizarDistancia(menor, actual);
		it = lista.begin();
		auxIt = it;
		++it;

		for(it; it != lista.end(); ++it){
			aux = *it;
			actualizarDistancia(aux, actual);
			if(aux.distanciaDestino < menor.distanciaDestino){
				menor = aux;
				auxIt = it;
			}	
		}

		lista.erase(auxIt);
		auxList.push_back(*auxIt);
	}

	lista = auxList;
}

//Funcion que comprueba si un estado está en una lista de estados
bool ComportamientoJugador::buscar(estado &st, list<estado> &lista){
	bool esta = false;
	list<estado>::iterator it;
	for(it = lista.begin(); it != lista.end() && !esta; ++it)
		if(st.fila == (*it).fila && st.columna == (*it).columna)
			esta = true;
	return esta;
}

//Funcion para saber si es necesario recargar
bool ComportamientoJugador::necesitaRecargar(Sensores sensores){
	return sensores.bateria < 1000;
}
