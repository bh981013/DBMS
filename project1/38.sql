select name
from Pokemon
where id in (select after_id
             from Evolution
             where after_id not in(select before_id from Evolution))
order by name;             