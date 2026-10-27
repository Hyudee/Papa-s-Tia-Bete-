#pragma once

// Constantes globais do jogo

#define COLUNAS      25
#define LINHAS       19
#define MAX_PROJETEIS 25
#define MAX_INIMIGOS  50
#define MAX_VIDAS     3
#define MAX_POWERUPS  2
#define MAX_POWERUPSATQ  1
#define MAX_OBSTACULOS 4
#define MAX_TIROS_INIMIGOS 8          // tiros do Pirulito na tela ao mesmo tempo
#define STUN_TIRO_INIMIGO 700        // ms de stun (e da animacao de dano) quando o tiro do Pirulito acerta o jogador
#define MAX_FAISCAS_ESCUDO 8          // faiscas de bloqueio do escudo do Brownie ao mesmo tempo
#define FPS_ALVO      60
#define INTERVALO_MINIMO_BOSS 10000   // ms entre o fim de um boss e o próximo
#define CARTAS_POR_OFERTA 3           // cartas mostradas depois de cada boss
#define COOLDOWN_TIRO_BASE 150        // ms mínimos entre dois disparos
#define ANGULO_LEQUE_DUPLO 5.0f       // graus de cada bala do Tiro Duplo em relação à mira
#define DANO_BOSS_BASE 3              // vidas que o boss tira ao chegar na base (cada carga de Barreira absorve 1)
#define GRUDENTO_COOLDOWN_PERCENT 140         // obstáculo grudento: tempo entre tiros x1,4 enquanto sujo
#define GRUDENTO_VELOCIDADE_TIRO_PERCENT 80   // obstáculo grudento: bala a 80% da velocidade enquanto sujo
#define MAX_TIROS_INIMIGOS 12