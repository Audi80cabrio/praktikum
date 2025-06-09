            p->round_total += card;
            if (p->round_total > 21) {
                p->done = true;
            }
        }

        if (!p->done) {
            p->wants_card = false;
        }

        player_ready[p->id] = true;
        responses++;
        pthread_cond_signal(&cond_bank);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* bank_thread(void* arg) {
    int round = 0;

    while (round < MAX_ROUNDS) {
        round++;
        printf("\n🃏 Runde %d\n", round);

        pthread_mutex_lock(&mutex);
        responses = 0;
        player_ready[0] = player_ready[1] = false;
                                                                                                      59,0-1        45%