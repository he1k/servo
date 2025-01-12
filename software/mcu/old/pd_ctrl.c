#include "pd.h"

void pd_init(, double T, double tau_d, double alpha){
  p->dummy1 = 0;
  p->dummy2 = 0;
  p->f0 = 0;
  p->b0 = 2*tau_d + T
  p->b1 = -2*tau_d + T
  p->a1 = -1;
}
void pd_out(pd_ctrl *p, double e){
  p->f0 = e - p->dummy1;
  p->u = p->b0 * p->f0 + p->dummy2;
}

void pd_update(pd_ctrl *p, double e){
  p->dummy1 = p->a1 * p->f0;
  p->dummy2 = p->b0 * p->f0;
}
