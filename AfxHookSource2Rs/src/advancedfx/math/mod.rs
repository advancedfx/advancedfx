pub const EPSILON: f64 = 1.0E-6f64;


#[repr(C)]
#[derive(Clone, Copy)]
pub struct Vector3 {
    x: f64,
    y: f64,
    z: f64,
}

impl Vector3 {
    pub fn new(x: f64, y: f64, z: f64) -> Vector3 {
        Self {
            x: x, y: y, z: z
        }
    }

    pub fn length(self) -> f64 {
        return (self.x*self.x + self.y*self.y * self.z*self.z).sqrt();
    }

    pub fn normalized(self) -> Self {
        self / self.length()
    }

    /**
    * @remarks Differs from C++ in that it is undefined for length 0.
    */
    pub fn normalize(&mut self) {
        *self = self.normalized()
    }
}

impl std::ops::Add for Vector3 {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        Self {x: self.x + other.x, y: self.y + other.y, z: self.z + other.z}
    }
}

impl std::ops::AddAssign for Vector3 {
    fn add_assign(&mut self, other: Self) {
        *self = Self {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
        };
    }
}

impl std::ops::Sub for Vector3 {
    type Output = Self;

    fn sub(self, other: Self) -> Self {
        Self {x: self.x - other.x, y: self.y - other.y, z: self.z - other.z}
    }
}


impl std::ops::SubAssign for Vector3 {
    fn sub_assign(&mut self, other: Self) {
        *self = Self {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
        };
    }
}

impl std::ops::Mul<f64> for Vector3 {
    type Output = Self;

    fn mul(self, rhs: f64) -> Self {
        Self {x: self.x * rhs, y: self.y * rhs, z: self.z * rhs}
    }
}

impl std::ops::MulAssign<f64> for Vector3 {
    fn mul_assign(&mut self, rhs: f64) {
        *self = *self * rhs;
    }
}

impl std::ops::Div<f64> for Vector3 {
    type Output = Self;

    fn div(self, rhs: f64) -> Self {
        Self {x: self.x / rhs, y: self.y / rhs, z: self.z / rhs}
    }
}

impl std::ops::DivAssign<f64> for Vector3 {
    fn div_assign(&mut self, rhs: f64) {
        *self = *self / rhs;
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct QEuelerAngles {
    pitch: f64,
    yaw: f64,
    roll: f64
}

impl QEuelerAngles {
    pub fn new(pitch: f64, yaw: f64, roll: f64) -> QEuelerAngles {
        Self {
            pitch: pitch, yaw: yaw, roll: roll
        }
    }
}

impl std::convert::From<QREulerAngles> for QEuelerAngles {
    fn from(item: QREulerAngles) -> Self {
        QEuelerAngles {
            pitch: 180.0 * item.pitch / std::f64::consts::PI,
            yaw: 180.0 * item.yaw / std::f64::consts::PI,
            roll: 180.0 * item.roll / std::f64::consts::PI
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct QREulerAngles {
    pitch: f64,
    yaw: f64,
    roll: f64,
}

impl QREulerAngles {
    pub fn new(pitch: f64, yaw: f64, roll: f64) -> QREulerAngles {
        Self {
            pitch: pitch, yaw: yaw, roll: roll
        }
    }
}

impl std::convert::From<QEuelerAngles> for QREulerAngles {
    fn from(item: QEuelerAngles) -> Self {
        QREulerAngles {
            pitch: std::f64::consts::PI * item.pitch / 180.0,
            yaw: std::f64::consts::PI * item.yaw / 180.0,
            roll: std::f64::consts::PI * item.roll / 180.0
        }
    }
}

impl std::convert::From<Quaternion> for QREulerAngles {
    fn from(item: Quaternion) -> Self {
        // TODO: There might still be a problem with singualrities in here!

        // Quaternion to matrix conversion taken from:
        // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

        // Quaternion to euler conversion analog (but changed) to:
        // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm

        let sqw = item.w*item.w;
        let sqx = item.x*item.x;
        let sqy = item.y*item.y;
        let sqz = item.z*item.z;

        let ssq = sqx + sqy + sqz + sqw;
        let invs = if 0.0 != ssq { 1.0 / ssq } else { 0.0 };
        let m00 = ( sqx - sqy - sqz + sqw)*invs;
        //let m11 = (-sqx + sqy - sqz + sqw)*invs;
        let m22 = (-sqx - sqy + sqz + sqw)*invs;
        
        let tmpa1 = item.x*item.y;
        let tmpa2 = item.z*item.w;
        let m10 = 2.0 * (tmpa1 + tmpa2)*invs;
        //let m01 = 2.0 * (tmpa1 - tmpa2)*invs;
        
        let tmpb1 = item.x*item.z;
        let tmpb2 = item.y*item.w;
        let m20 = 2.0 * (tmpb1 - tmpb2)*invs;
        //let m02 = 2.0 * (tmpb1 + tmpb2)*invs;

        let tmpc1 = item.y*item.z;
        let tmpc2 = item.x*item.w;
        let m21 = 2.0 * (tmpc1 + tmpc2)*invs;
        //let m12 = 2.0 * (tmpc1 - tmpc2)*invs;

        // X =            Y =            Z =
        // |1, 0 , 0  | |cp , 0, sp| |cy, -sy, 0|
        // |0, cr, -sr| |0  , 1, 0 | |sy, cy , 0|
        // |0, sr, cr | |-sp, 0, cp| |0 , 0  , 1|

        // Y*X =
        // |cp , sp*sr, sp*cr|
        // |0  , cr   , -sr  |
        // |-sp, cp*sr, cp*cr|

        // Z*(Y*X) =
        // |cy*cp, cy*sp*sr -sy*cr, cy*sp*cr +sy*sr |
        // |sy*cp, sy*sp*sr +cy*cr, sy*sp*cr +cy*-sr|
        // |-sp  , cp*sr          , cp*cr           |

        // 1) cy*cp = m00
        // 2) cy*sp*sr -sy*cr = m01
        // 3) cy*sp*cr +sy*sr = m02
        // 4) sy*cp = m10
        // 5) sy*sp*sr +cy*cr = m11
        // 6) sy*sp*cr +cy*-sr = m12
        // 7) -sp = m20
        // 8) cp*sr = m21
        // 9) cp*cr = m22
        //
        // 7=> p = arcsin( -m20 )
        //
        // 4/1=> y = arctan2( m10, m00 )
        //
        // 8/9=> r = arctan2( m21, m22 )
        
        let sin_y_pitch = -m20;

        if sin_y_pitch > 1.0 -EPSILON {
            // sout pole singularity:
            QREulerAngles {
                pitch: std::f64::consts::PI / 2.0, // Y
                yaw: 0.0, // Z
                roll:  - 2.0 * (item.z*invs).atan2(item.w*invs)  // X
            }

        } else if sin_y_pitch < -1.0 +EPSILON {
            // north pole singularity:
            QREulerAngles {
                pitch: - std::f64::consts::PI / 2.0, // Y
                yaw: 0.0, // Z
                roll:  2.0 * (item.z*invs).atan2(item.w*invs) // X
            }

        } else {
            // hopefully no singularity:
            QREulerAngles {
                pitch: sin_y_pitch.asin(), // Y
                yaw: m10.atan2( m00 ), // Z
                roll:  m21.atan2( m22 )  // X
            }            
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Quaternion {
    w: f64,
    x: f64,
    y: f64,
    z: f64,
}

impl Quaternion {
    pub fn unit() -> Quaternion {
        Self {
            w: 1.0, x: 0.0, y: 0.0, z: 0.0
        }   
    }

    pub fn new(w: f64, x: f64, y: f64, z: f64) -> Quaternion {
        Self {
            w: w, x: x, y: y, z: z
        }
    }

    pub fn dot(self, rhs: Self) -> f64 {
        return self.w*rhs.w + self.x*rhs.x +self.y*rhs.y +self.z*rhs.z;
    }

    pub fn norm(self) -> f64 {
        (self.w * self.w + self.x * self.x + self.y * self.y + self.z * self.z).sqrt()
    }

    pub fn normalized(self) -> Self {
        let norm = self.norm();
        Quaternion { w: self.w / norm, x: self.x / norm, y: self.y / norm, z: self.z / norm }
    }

    pub fn normalize(&mut self) {
        *self = self.normalized()
    }

    pub fn conjugate(self) -> Self {
        Self {
            w: self.w, x: -self.x, y: -self.y, z: -self.z
        }
    }

    pub fn get_ang(self, rhs: Self) -> (Vector3,f64) {

        let mut temp = Vector3 {
            x: self.w*rhs.x - self.x*rhs.w - self.y*rhs.z + self.z*rhs.y,
            y: self.w*rhs.y - self.y*rhs.w - self.z*rhs.x + self.x*rhs.z,
            z: self.w*rhs.z - self.z*rhs.w - self.x*rhs.y + self.y*rhs.x
        };

        let ca = self.x*rhs.x + self.y*rhs.y + self.z*rhs.z + self.w*rhs.w;

        let sa = temp.length();
        if 0.0 < sa { temp = temp / sa };
    
        let dtheta = sa.atan2(ca) * 2.0;

        (temp, dtheta)
    }

    pub fn slerp(self, rhs: Self, t: f64) -> Self {
        let (eigen_axis, dtheta) = self.get_ang(rhs);
        let t_dtheta_div_2 = t * dtheta / 2.0;
        let cos = t_dtheta_div_2.cos();
        let sin = t_dtheta_div_2.sin();

        self * Quaternion {
            w: cos, x: eigen_axis.x * sin, y: eigen_axis.y * sin, z: eigen_axis.z * sin
        }        
    }
}

impl std::ops::Add for Quaternion {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        Self {w: self.w + other.w, x: self.x + other.x, y: self.y + other.y, z: self.z + other.z}
    }
}

impl std::ops::Sub for Quaternion {
    type Output = Self;

    fn sub(self, other: Self) -> Self {
        Self {w: self.w - other.w, x: self.x - other.x, y: self.y - other.y, z: self.z - other.z}
    }
}  

impl std::ops::Mul<Quaternion> for f64 {
    type Output = Quaternion;

    fn mul(self, rhs: Quaternion) -> Self::Output {
        Quaternion { 
            w: self*rhs.w,
            x: self*rhs.x,
            y: self*rhs.y,
            z: self*rhs.z
        }
    }
}

impl std::ops::Mul for Quaternion {
    type Output = Self;

    fn mul(self, rhs: Quaternion) -> Self {
        Self {
            w: self.w*rhs.w - self.x*rhs.x - self.y*rhs.y - self.z*rhs.z,
            x: self.w*rhs.x + self.x*rhs.w + self.y*rhs.z - self.z*rhs.y,
            y: self.w*rhs.y - self.x*rhs.z + self.y*rhs.w + self.z*rhs.x,
            z: self.w*rhs.z + self.x*rhs.y - self.y*rhs.x + self.z*rhs.w                    
        }
    }
}        

impl std::convert::From<QREulerAngles> for Quaternion {
    fn from(item: QREulerAngles) -> Self {
        // todo: this can be optimized (since many components are 0),
        // but there was a bug in it, so let's do it inefficiently for now:

        let pitch_h: f64 = 0.5 * item.pitch;
        let q_pitch_y = Quaternion::new(pitch_h.cos(), 0.0, pitch_h.sin(), 0.0);
        
        let yaw_h: f64 = 0.5 * item.yaw;
        let q_yaw_z = Quaternion::new(yaw_h.cos(), 0.0, 0.0, yaw_h.sin());
        
        let roll_h: f64 = 0.5 * item.roll;
        let q_roll_x = Quaternion::new(roll_h.cos(), roll_h.sin(), 0.0, 0.0);
        
        q_yaw_z * q_pitch_y * q_roll_x
    }
}
