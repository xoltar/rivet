#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

//forward declarations
class InputManager;
struct InputParameters;
class Mesh;

#include "dcel/barcode_template.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"

#include <QObject>
#include <QThread>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

#include "dcel/mesh_message.h"
#include <vector>

class ComputationThread : public QThread {
    Q_OBJECT

    friend class InputManager; //so that we don't have to pass all of the data structures from ComputationThread to InputManager

public:
    ComputationThread(InputParameters& params,
        QObject* parent = 0);
    ~ComputationThread();

    void compute();

    //TODO: these really ought to be delivered via signal rather than read by other classes
    XiSupportMessage message;
    std::vector<xiPoint> xi_support;
    std::vector<exact> x_exact;
    std::vector<exact> y_exact;
    unsigned_matrix hom_dims;
    QString x_label;
    QString y_label;

signals:
    void advanceProgressStage();
    void setProgressMaximum(unsigned max);
    void setCurrentProgress(unsigned current);
    void xiSupportReady();
    void arrangementReady(MeshMessage* arrangement);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    InputParameters& params;

    std::shared_ptr<MeshMessage> arrangement;

    void compute_from_file();
    void unpack_message_fields();
    bool is_precomputed(std::string file_name);
    void load_from_file();
};

//TODO: Move this somewhere. See comments on implementation for details.
void write_boost_file(QString file_name, InputParameters const& params, XiSupportMessage const& message, MeshMessage const& mesh);
#endif // COMPUTATIONTHREAD_H
