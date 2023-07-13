  
  void DrawGrid(const float* view,
                const float* projection,
                const float* matrix,
                const float gridSize)
  {
    matrix_t viewProjection = *(matrix_t*)view * *(matrix_t*)projection;
    vec_t frustum[6];
    ComputeFrustumPlanes(frustum, &viewProjection.m00);
    matrix_t res = *(matrix_t*)matrix * viewProjection;

    for (float f = -gridSize; f <= gridSize; f += 1.f) {
      for (int dir = 0; dir < 2; dir++) {
        vec_t ptA = { dir ? -gridSize : f, 0.f, dir ? f : -gridSize };
        vec_t ptB = { dir ? gridSize : f, 0.f, dir ? f : gridSize };
        bool visible = true;
        for (int i = 0; i < 6; i++) {
          float dA = DistanceToPlane(ptA, frustum[i]);
          float dB = DistanceToPlane(ptB, frustum[i]);
          if (dA < 0.f && dB < 0.f) {
            visible = false;
            break;
          }
          if (dA > 0.f && dB > 0.f) {
            continue;
          }
          if (dA < 0.f) {
            float len = fabsf(dA - dB);
            float t = fabsf(dA) / len;
            ptA.Lerp(ptB, t);
          }
          if (dB < 0.f) {
            float len = fabsf(dB - dA);
            float t = fabsf(dB) / len;
            ptB.Lerp(ptA, t);
          }
        }
        if (visible) {
          ImU32 col = IM_COL32(0x80, 0x80, 0x80, 0xFF);
          col = (fmodf(fabsf(f), 10.f) < FLT_EPSILON)
                  ? IM_COL32(0x90, 0x90, 0x90, 0xFF)
                  : col;
          col =
            (fabsf(f) < FLT_EPSILON) ? IM_COL32(0x40, 0x40, 0x40, 0xFF) : col;

          float thickness = 1.f;
          thickness = (fmodf(fabsf(f), 10.f) < FLT_EPSILON) ? 1.5f : thickness;
          thickness = (fabsf(f) < FLT_EPSILON) ? 2.3f : thickness;

          mDrawList->AddLine(
            worldToPos(ptA, res), worldToPos(ptB, res), col, thickness);
        }
      }
    }
  }

