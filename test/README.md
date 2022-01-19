# Testing Cases

Samples programs are categorized according to their domains:

## Linear Algebra
**Matmul**
```c++
for (i: 0 -> M) {
    for (j: 0 -> N) {
        for (k: 0 -> K) {
            C(i, j) += A(i, k) * B(k, j)
        }
    }
}
```
**LU Decomposition**
```c++
for (i: 0 -> n) {
    U(i, i) = 1
}
for (i: 0 -> n) {
    for (j : i -> n) {
        L(j, i) = A(j, i);
        for (k = 0; k < i; k++) {
            L(j, i) = L(j, i) - L(j, k) * U(k, i);
        }
    }
    for (j: i + 1 -> n) {
        U(i, j) = A(i, j) / L(i, i);
        for (k = 0; k < i; k++) {
            U(i, j) = U(i, j) - ((L(i, k) * U(k, j)) / L(i, i));
        }
    }
}
```
**Cholesky Decomposition**
```c++
for (j: 0 -> N) {
    d = .0
    for (k: 0 -> j) {
        s = .0
        for (i: 0 -> k) {
            s += L(k, i) * L(j, i)
        }
        s = (A(j, k) - s) / L(k, k)
        L(j, k) = s
        d = d + s*s
    }
    d = A(j, j) - d
    L(j, j) = sqrt(max(0, d))
    for (k: j + 1 -> n) {
        L(j, k) = 0
    }
}
```
**QR Decomposition (Eigen Value)**
```c++
```
**HouseHolder Transformation**
```c++
```
**Givens Transformation**
```c++
```
## Computer Graphics
**Blurring (3x3 unnormalized box filter)**
```c++
for (h: 0 -> H) {
    for (w: 0 -> W) {
        for (i: -1 -> 1) {
            Blur(h, w) += I(h - i, w)
        }
    }
}
for (h: 0 -> H) {
    for (w: 0 -> W) {
        for (i: -1 -> 1) {
            O(h, w) += Blur(h, w + i)
        }
    }
}
```
**Gaussian Pyramid**
- Reduce
```c++
for (n: 0 -> L) {
    for (h: 0 -> H) {
        for (w: 0 -> W) {
            for (i: -2 -> 2) {
                for (j: -2 -> 2) {
                    G(n + 1, h, w) += G(n, 2h - i, 2w - j) * W(i, j)
                }
            }
        }
    }
}
```
- Expand
```c++
for (n: L -> 0) {
    for (h: 0 -> H) {
        for (w: 0 -> W) {
            for (i: -2 -> 2) {
                for (j: -2 -> 2) {
                    G(n, h, w) += 4 * G(n + 1, (h - i) / 2, (w - j) / 2) * W(i, j)
                }
            }
        }
    }
}
```
**Local Laplacian Filter**
```c++

```

## Machine/Deep Learning

**Convolution**
```c++
sum = .0
for (i: 0 -> BATCH) {
    for (c: 0 -> CHANNEL) {
        for (h: 0 -> H - KH) {
            for (w: 0 -> W - KW) {
                for (Kh: 0 -> KH) {
                    for (Kw: 0 -> KW) {
                        O(i, c, h, w) += K(Kh, Kw) * I(i, c, h + Kh, w + Kw)
                    }
                }
            }
        }
    }
}
```
**Softmax**
```c++
sum = .0
for (i: 0 -> N) {
    O(i) = exp(I(i))
    sum += O(i)
}
for (i: 0 -> N) {
    O(i) = O(i) / sum
}
```

## Scientific Computation
**2D Stencil**
```c++
for (i : 0 -> M) {
    for (j : 0 -> N) {
        T(i, j) = 0.25 ( S(i - 1, j) + S(i + 1, j) + S(i, j - 1) + S(i, j + 1))
    }
}
```
