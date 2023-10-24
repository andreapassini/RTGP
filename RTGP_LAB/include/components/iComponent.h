class IComponent
{
public:
    virtual void TimeStep() = 0;
    bool isActive;
};
