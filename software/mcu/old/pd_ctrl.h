typedef struct{
  double dummy1, dummy2, f0, u, b0, b1, a1;
}pd_ctrl;

void pd_init(pd_ctrl *p, double T, double tau_d, double alpha);
void pd_out(pd_ctrl *p, double e);
void pd_update(pd_ctrl *p, double e);
