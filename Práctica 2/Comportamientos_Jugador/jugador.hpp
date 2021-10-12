#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
  int fila = -1;
  int columna = -1;
  int orientacion = -1;
  int costeAcumulado = 0; //g(n)
  bool bikini = false;
  bool zapatillas = false;
  int distanciaDestino = 0; //h(n)
  int estimacionCosteTotal = 0; //f(n)
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      hayPlan = false;  
      pasosDados = 0;
      instantesConsumidos = 0; 
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      hayPlan = false;
      pasosDados = 0;
      instantesConsumidos = 0; 
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

    int calcularCosteCasilla(estado &st, const vector<vector<unsigned char>> &mapa, Action accion);

    void calcularConsumo(const estado &padre, estado &hijo, Action accion);

    void actualizarEstado(estado &st);

  private:
    // Declarar Variables de Estado
    estado actual;
    list<estado> objetivos;
    list<Action> plan;
    bool hayPlan;
    int pasosDados;
    list<estado> objetivosPendientes;
    list<estado> bateriasEncontradas;
    int instantesConsumidos;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_MenorConsumo(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_AEstrella(const estado &origen, const estado &destino, list<Action> &plan);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);

    void pintarMapa(Sensores sensor);

    void actualizarDistancia(estado &st, const estado &destino);
    void actualizarEstimacionCoste(estado &st);

    void ordenarLista(list<estado> &lista);

    bool necesitaRecargar(Sensores sensores);

    bool buscar(estado &st, list<estado> &lista);
};

#endif
