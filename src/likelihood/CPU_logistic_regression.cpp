#include "CPU_logistic_regression.hpp"

double CPU_LogisticRegression::train(int n_iter,double alpha,double tol){
	VectorXd log_likelihood=VectorXd::Zero(n_iter);
	for(int i=0;i<n_iter;i++){
		//tools.printProgBar(i, n_iter);
		this->preCompute();
		log_likelihood(i)=-this->logPosterior();
		//if (i % 10 == 0) cout << "iteration :   " << i << " | loss : " << log_likelihood(i) << endl;
		VectorXd gradient=this->computeGradient();
		this->momemtum*=alpha;
		this->momemtum-=tol*gradient/(double)this->rows;
		this->weights+=this->momemtum;
		if(this->with_bias) this->bias-=tol*this->grad_bias/(double)this->rows;
	}
	return log_likelihood.mean();
}

void CPU_LogisticRegression::preCompute(){
	//cout << this->weights.array().isFinite().transpose() << endl;
	this->eta = (*this->X_train * this->weights);
	if(this->with_bias) this->eta.noalias()=(this->eta.array()+this->bias).matrix();
	this->phi = this->sigmoid(this->eta);
}

VectorXd CPU_LogisticRegression::computeGradient(){
	VectorXd E_d=this->phi-*this->Y_train;
	VectorXd E_w=this->weights/(this->lambda);
	VectorXd grad=VectorXd::Zero(this->dim);
	#pragma omp parallel for schedule(static)
	for(int d=0;d<this->dim;d++){
		grad[d]=this->X_train->col(d).cwiseProduct(E_d).sum()+E_w[d];
	}
	if(this->with_bias) this->grad_bias= (E_d.sum()+this->bias/this->lambda);
	return grad;
}

VectorXd CPU_LogisticRegression::predict(MatrixXd &_X_test,bool prob, bool data_processing){
	if (data_processing){
		if (this->normalization) tools.testNormalization(_X_test,this->featureMax,this->featureMin);
		if(this->standardization) tools.testStandardization(_X_test,this->featureMean,this->featureStd);
	}
	VectorXd eta_test = (_X_test)*this->weights;
	if(this->with_bias) eta_test.noalias()=(eta_test.array()+this->bias).matrix();
	VectorXd phi_test=this->sigmoid(eta_test);
	if(!prob){
		phi_test.noalias() = phi_test.unaryExpr([](double elem){
	    	return (elem > 0.5) ? 1.0 : 0.0;
		});		
	}
	return phi_test;
}
