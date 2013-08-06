class C3DModel : public C3dsFileLoader
{
public:
	C3DModel() : C3dsFileLoader() {}
protected:
	void		User3dVert(float x, float y, float z);
	void		User3dFace(unsigned short A, unsigned short B, 
				    unsigned short C, unsigned short Flags);
	void		UserCamera(float px, float py, float pz,
			 	    float tx, float ty, float tz,
				    float Bank, float Lens);
	void		UserMapVertex(float U, float V);
	void		UserTransformMatrix(const Transform3dsMatrix &Transform, 
					      const Translate3dsMatrix &Translate);
	void		UserChunkObj();
	void		UserEndOfFile();
};
