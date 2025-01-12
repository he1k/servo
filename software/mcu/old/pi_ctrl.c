#include "pi.h"

void pi_init(pi_ctrl *p){
  p->dummy1 = 0;
  p->dummy2 = 0;
  p->f0 = 0;
  p->b0 = 1 + T/(2*tau_i);
  p->b1 = -1 + T/(2*tau_i);
  p->a1 = -1;
}
void pi_out(pi_ctrl *p, double e){
  p->f0 = e - p->dummy1;
  p->u = p->b0 * p->f0 + p->dummy2;
}

void pi_update(pi_ctrl *p, double e){
  p->dummy1 = p->a1 * p->f0;
  p->dummy2 = p->b0 * p->f0;
}
