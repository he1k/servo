typedef struct{
  double dummy1, dummy2, f0, u, b0, b1, a1;
}pi_ctrl;

void pi_init(pi_ctrl *p);
void pi_out(pi_ctrl *p, double e);
void pi_update(pi_ctrl *p, double e);
