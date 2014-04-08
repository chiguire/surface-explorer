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
      for (int j = 0; j != surfacePointsH; j++) {
        for (int i = 0; i != surfacePointsW; i++) {
          surfacePoints[j*surfacePointsW+i][1] = evaluate(surfacePoints[j*surfacePointsW+i]);
        }
      }
    }

    float evaluate(vec3 &p) {
      //find control point neighbors of p and do bilinear interpolation
      vec3 initPoint = (*controlPoints)[0];

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
      return p2.y();
    }

    void get_points(dynarray<float> &result) {
      result.reset();

      for (int i = 0; i != surfacePointsW*surfacePointsH; i++) {
        result.push_back(surfacePoints[i].x());
        result.push_back(surfacePoints[i].y());
        result.push_back(surfacePoints[i].z());
      }
    }

    unsigned int get_width() { return surfacePointsW; }
    unsigned int get_height() { return surfacePointsH; }
  };

}