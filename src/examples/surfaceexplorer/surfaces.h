namespace octet {

  class linear_surface {
  private:
    dynarray <vec3> *controlPoints;
    dynarray <vec3> surfacePoints;

    unsigned int controlPointsW;
    unsigned int controlPointsH;
    
    float controlPointsStepX;
    float controlPointsStepZ;

    unsigned int surfacePointsW;
    unsigned int surfacePointsH;

  public:
    linear_surface()
    : controlPoints(NULL)
    , surfacePoints()
    , controlPointsW(0)
    , controlPointsH(0)
    {
    }

    void init(dynarray<vec3> *controlPoints_, unsigned int cPWidth, unsigned int cPHeight, unsigned int sPWidth, unsigned int sPHeight) {
      controlPoints = controlPoints_;
      controlPointsW = cPWidth;
      controlPointsH = cPHeight;
      
      surfacePoints.resize(sPWidth*sPHeight);
      surfacePointsW = sPWidth;
      surfacePointsH = sPHeight;

      vec3 extremes[4] = { (*controlPoints)[0], (*controlPoints)[cPWidth-1], (*controlPoints)[(cPHeight-1)*cPWidth], (*controlPoints)[cPHeight*cPWidth-1] };
      vec3 diff = (extremes[3] - extremes[0]);
      diff[0] /= sPWidth;
      diff[2] /= sPHeight;

      controlPointsStepX = ((*controlPoints)[1] - extremes[0]).x();
      controlPointsStepZ = ((*controlPoints)[cPWidth] - extremes[0]).z();

      for (int j = 0; j != sPHeight; j++) {
        for (int i = 0; i != sPWidth; i++) {
          surfacePoints[j*sPHeight+i][0] = extremes[0].x()+diff[0]*i;
          surfacePoints[j*sPHeight+i][1] = 0;
          surfacePoints[j*sPHeight+i][2] = extremes[0].z()+diff[2]*j;
        }
      }
    }

    void update() {

      float tValue = 1/15.0f;

      for (int i = 0; i != surfacePointsH; i++) {
        for (int j = 0; j != surfacePointsW; j++) {
          surfacePoints[i*surfacePointsH+j][1] = evaluate(surfacePoints[i*surfacePointsH+j],i*tValue,j*tValue);
        }
      }
    }

    float evaluate(vec3 &p, float u, float v) {
      //find control point neighbors of p and do bilinear interpolation
      /*vec3 initPoint = (*controlPoints)[0];

      float x = p.x();
      float z = p.z();

      int stepX = min((int)((x-initPoint.x())/controlPointsStepX), (int)(controlPointsW-2));
      int stepZ = min((int)((z-initPoint.z())/controlPointsStepZ), (int)(controlPointsH-2));

      vec3 controlPointNeighbors[4] = { (*controlPoints)[stepZ*controlPointsW+stepX], (*controlPoints)[stepZ*controlPointsW+stepX+1],
      (*controlPoints)[(stepZ+1)*controlPointsW+stepX], (*controlPoints)[(stepZ+1)*controlPointsW+stepX+1] };

      float u = x-controlPointNeighbors[0].x();
      float v = z-controlPointNeighbors[0].z();

      vec3 p0(controlPointNeighbors[0]+(controlPointNeighbors[1]-controlPointNeighbors[0])*u);
      vec3 p1(controlPointNeighbors[2]+(controlPointNeighbors[3]-controlPointNeighbors[2])*u);
      vec3 p2(p0+(p1-p0)*v);
      return p2.y();*/

      float sum = 0.0f;

      for(int i=0; i!=4; ++i){
        for(int j=0; j!=4; ++j){

          vec3 controlPoint = (*controlPoints)[i*4+j];

          sum+= controlPoint.y()*bernsteinPolynomial(i,u)*bernsteinPolynomial(j,v);

        }
      }

      return sum;

    }

    float bernsteinPolynomial(int grade, float t){
      if(grade == 0){
        return ((1-t)*(1-t)*(1-t));
      }else if(grade == 1){
        return (3*t*(1-t)*(1-t));
      }else if(grade == 2){
        return (3*t*t*(1-t));
      }else if(grade == 3){
        return (t*t*t);
      }
    }

    void get_points(dynarray<float> &result) {
      result.reset();

      for (int i = 0; i != surfacePointsW*surfacePointsH; i++) {
        result.push_back(surfacePoints[i].x());
        result.push_back(surfacePoints[i].y());
        result.push_back(surfacePoints[i].z());
      }
    }

    vec3 getPointAt(int i, int j){
      return surfacePoints[i*surfacePointsH+j];
    }

    unsigned int get_width() { return surfacePointsW; }
    unsigned int get_height() { return surfacePointsH; }


    void resetAllPoints(){
      for(int i=0;i!=controlPoints->size();++i){
        (*controlPoints)[i][1]=0.0f;
      }
    }
  };

}